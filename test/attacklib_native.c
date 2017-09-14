#include <Python.h>
#include <frameobject.h>
#include <opcode.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static PyObject * replace_print(PyObject *self, PyObject *args) {
    PyFrameObject *f = PyEval_GetFrame();
    Py_INCREF(f);

    PyFrame_FastToLocals(f);

    // Replace the value of the local python variable p
    PyMapping_SetItemString(f->f_locals, "p", PyUnicode_FromString("pwned 5: Replaced the function param from a native lib"));

    // Merge it back with fast so the change persists
    PyFrame_LocalsToFast(f, 0);

    Py_DECREF(f);
    Py_INCREF(Py_None);
    return Py_None;
}

static int
frame_setlineno(PyFrameObject *f, PyObject* p_new_lineno)
{
    int new_lineno = 0;                 /* The new value of f_lineno */
    long l_new_lineno;
    int overflow;
    int new_lasti = 0;                  /* The new value of f_lasti */
    int new_iblock = 0;                 /* The new value of f_iblock */
    unsigned char *code = NULL;         /* The bytecode for the frame... */
    Py_ssize_t code_len = 0;            /* ...and its length */
    unsigned char *lnotab = NULL;       /* Iterating over co_lnotab */
    Py_ssize_t lnotab_len = 0;          /* (ditto) */
    int offset = 0;                     /* (ditto) */
    int line = 0;                       /* (ditto) */
    int addr = 0;                       /* (ditto) */
    int min_addr = 0;                   /* Scanning the SETUPs and POPs */
    int max_addr = 0;                   /* (ditto) */
    int delta_iblock = 0;               /* (ditto) */
    int min_delta_iblock = 0;           /* (ditto) */
    int min_iblock = 0;                 /* (ditto) */
    int f_lasti_setup_addr = 0;         /* Policing no-jump-into-finally */
    int new_lasti_setup_addr = 0;       /* (ditto) */
    int blockstack[CO_MAXBLOCKS];       /* Walking the 'finally' blocks */
    int in_finally[CO_MAXBLOCKS];       /* (ditto) */
    int blockstack_top = 0;             /* (ditto) */
    unsigned char setup_op = 0;         /* (ditto) */

    /* f_lineno must be an integer. */
    if (!PyLong_CheckExact(p_new_lineno)) {
        PyErr_SetString(PyExc_ValueError,
                        "lineno must be an integer");
        return -1;
    }

    /* Fail if the line comes before the start of the code block. */
    l_new_lineno = PyLong_AsLongAndOverflow(p_new_lineno, &overflow);
    if (overflow
#if SIZEOF_LONG > SIZEOF_INT
        || l_new_lineno > INT_MAX
        || l_new_lineno < INT_MIN
#endif
       ) {
        PyErr_SetString(PyExc_ValueError,
                        "lineno out of range");
        return -1;
    }
    new_lineno = (int)l_new_lineno;

    if (new_lineno < f->f_code->co_firstlineno) {
        PyErr_Format(PyExc_ValueError,
                     "line %d comes before the current code block",
                     new_lineno);
        return -1;
    }
    else if (new_lineno == f->f_code->co_firstlineno) {
        new_lasti = 0;
        new_lineno = f->f_code->co_firstlineno;
    }
    else {
        /* Find the bytecode offset for the start of the given
         * line, or the first code-owning line after it. */
        char *tmp;
        PyBytes_AsStringAndSize(f->f_code->co_lnotab,
                                &tmp, &lnotab_len);
        lnotab = (unsigned char *) tmp;
        addr = 0;
        line = f->f_code->co_firstlineno;
        new_lasti = -1;
        for (offset = 0; offset < lnotab_len; offset += 2) {
            addr += lnotab[offset];
            line += lnotab[offset+1];
            if (line >= new_lineno) {
                new_lasti = addr;
                new_lineno = line;
                break;
            }
        }
    }

    /* If we didn't reach the requested line, return an error. */
    if (new_lasti == -1) {
        PyErr_Format(PyExc_ValueError,
                     "line %d comes after the current code block",
                     new_lineno);
        return -1;
    }

    /* We're now ready to look at the bytecode. */
    PyBytes_AsStringAndSize(f->f_code->co_code, (char **)&code, &code_len);
    min_addr = Py_MIN(new_lasti, f->f_lasti);
    max_addr = Py_MAX(new_lasti, f->f_lasti);

    /* You can't jump onto a line with an 'except' statement on it -
     * they expect to have an exception on the top of the stack, which
     * won't be true if you jump to them.  They always start with code
     * that either pops the exception using POP_TOP (plain 'except:'
     * lines do this) or duplicates the exception on the stack using
     * DUP_TOP (if there's an exception type specified).  See compile.c,
     * 'com_try_except' for the full details.  There aren't any other
     * cases (AFAIK) where a line's code can start with DUP_TOP or
     * POP_TOP, but if any ever appear, they'll be subject to the same
     * restriction (but with a different error message). */
    if (code[new_lasti] == DUP_TOP || code[new_lasti] == POP_TOP) {
        PyErr_SetString(PyExc_ValueError,
            "can't jump to 'except' line as there's no exception");
        return -1;
    }

    /* You can't jump into or out of a 'finally' block because the 'try'
     * block leaves something on the stack for the END_FINALLY to clean
     * up.      So we walk the bytecode, maintaining a simulated blockstack.
     * When we reach the old or new address and it's in a 'finally' block
     * we note the address of the corresponding SETUP_FINALLY.  The jump
     * is only legal if neither address is in a 'finally' block or
     * they're both in the same one.  'blockstack' is a stack of the
     * bytecode addresses of the SETUP_X opcodes, and 'in_finally' tracks
     * whether we're in a 'finally' block at each blockstack level. */
    f_lasti_setup_addr = -1;
    new_lasti_setup_addr = -1;
    memset(blockstack, '\0', sizeof(blockstack));
    memset(in_finally, '\0', sizeof(in_finally));
    blockstack_top = 0;
    for (addr = 0; addr < code_len; addr++) {
        unsigned char op = code[addr];
        switch (op) {
        case SETUP_LOOP:
        case SETUP_EXCEPT:
        case SETUP_FINALLY:
        case SETUP_WITH:
        case SETUP_ASYNC_WITH:
            blockstack[blockstack_top++] = addr;
            in_finally[blockstack_top-1] = 0;
            break;

        case POP_BLOCK:
            assert(blockstack_top > 0);
            setup_op = code[blockstack[blockstack_top-1]];
            if (setup_op == SETUP_FINALLY || setup_op == SETUP_WITH
                                    || setup_op == SETUP_ASYNC_WITH) {
                in_finally[blockstack_top-1] = 1;
            }
            else {
                blockstack_top--;
            }
            break;

        case END_FINALLY:
            /* Ignore END_FINALLYs for SETUP_EXCEPTs - they exist
             * in the bytecode but don't correspond to an actual
             * 'finally' block.  (If blockstack_top is 0, we must
             * be seeing such an END_FINALLY.) */
            if (blockstack_top > 0) {
                setup_op = code[blockstack[blockstack_top-1]];
                if (setup_op == SETUP_FINALLY || setup_op == SETUP_WITH
                                    || setup_op == SETUP_ASYNC_WITH) {
                    blockstack_top--;
                }
            }
            break;
        }

        /* For the addresses we're interested in, see whether they're
         * within a 'finally' block and if so, remember the address
         * of the SETUP_FINALLY. */
        if (addr == new_lasti || addr == f->f_lasti) {
            int i = 0;
            int setup_addr = -1;
            for (i = blockstack_top-1; i >= 0; i--) {
                if (in_finally[i]) {
                    setup_addr = blockstack[i];
                    break;
                }
            }

            if (setup_addr != -1) {
                if (addr == new_lasti) {
                    new_lasti_setup_addr = setup_addr;
                }

                if (addr == f->f_lasti) {
                    f_lasti_setup_addr = setup_addr;
                }
            }
        }

        if (op >= HAVE_ARGUMENT) {
            addr += 2;
        }
    }

    /* Verify that the blockstack tracking code didn't get lost. */
    assert(blockstack_top == 0);

    /* After all that, are we jumping into / out of a 'finally' block? */
    if (new_lasti_setup_addr != f_lasti_setup_addr) {
        PyErr_SetString(PyExc_ValueError,
                    "can't jump into or out of a 'finally' block");
        return -1;
    }


    /* Police block-jumping (you can't jump into the middle of a block)
     * and ensure that the blockstack finishes up in a sensible state (by
     * popping any blocks we're jumping out of).  We look at all the
     * blockstack operations between the current position and the new
     * one, and keep track of how many blocks we drop out of on the way.
     * By also keeping track of the lowest blockstack position we see, we
     * can tell whether the jump goes into any blocks without coming out
     * again - in that case we raise an exception below. */
    delta_iblock = 0;
    for (addr = min_addr; addr < max_addr; addr++) {
        unsigned char op = code[addr];
        switch (op) {
        case SETUP_LOOP:
        case SETUP_EXCEPT:
        case SETUP_FINALLY:
        case SETUP_WITH:
        case SETUP_ASYNC_WITH:
            delta_iblock++;
            break;

        case POP_BLOCK:
            delta_iblock--;
            break;
        }

        min_delta_iblock = Py_MIN(min_delta_iblock, delta_iblock);

        if (op >= HAVE_ARGUMENT) {
            addr += 2;
        }
    }

    /* Derive the absolute iblock values from the deltas. */
    min_iblock = f->f_iblock + min_delta_iblock;
    if (new_lasti > f->f_lasti) {
        /* Forwards jump. */
        new_iblock = f->f_iblock + delta_iblock;
    }
    else {
        /* Backwards jump. */
        new_iblock = f->f_iblock - delta_iblock;
    }

    /* Are we jumping into a block? */
    if (new_iblock > min_iblock) {
        PyErr_SetString(PyExc_ValueError,
                        "can't jump into the middle of a block");
        return -1;
    }

    /* Pop any blocks that we're jumping out of. */
    while (f->f_iblock > new_iblock) {
        PyTryBlock *b = &f->f_blockstack[--f->f_iblock];
        while ((f->f_stacktop - f->f_valuestack) > b->b_level) {
            PyObject *v = (*--f->f_stacktop);
            Py_DECREF(v);
        }
    }

    /* Finally set the new f_lineno and f_lasti and return OK. */
    f->f_lineno = new_lineno;
    f->f_lasti = new_lasti;
    return 0;
}

static PyObject * jump_print(PyObject *self, PyObject *args) {

    PyFrameObject *f = PyEval_GetFrame();
    Py_INCREF(f);

    // Change the next line number in the frame to exec
    if (frame_setlineno(f, PyLong_FromLong(58))) {
        return NULL;
    }

    // subtract 1, so the interpreter actually computes the
    // correct next instruction at the next eval iteration
    f->f_lasti--;

    Py_DECREF(f);
    Py_INCREF(Py_None);
    return Py_None;
}

static const char moduledocstring[] = "Frame hacking from native lib prototype";

PyMethodDef al_methods[] = {
    {"replace", (PyCFunction)replace_print, METH_VARARGS, "Replaces the value of the function arg with pwned"},
    {"jump", (PyCFunction)jump_print, METH_VARARGS, "Jumps to line 54 in the current stack frame"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef almodule = {
   PyModuleDef_HEAD_INIT,
   "attacklib_native",      // name of module
   moduledocstring,  // module documentation, may be NULL
   -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   al_methods
};

PyMODINIT_FUNC
PyInit_attacklib_native(void) {
    return PyModule_Create(&almodule);
}
