#include <Python.h>
#include <frameobject.h>


static void bad_open(void) {
    printf("\n\nAey,\nWhat's up doc :) \n\n");
}

static void foo(char *input) {
    char buffer[14];
    strcpy(buffer, input);
}


static PyObject * native_print(PyObject *self, PyObject *args) {
    const char *str;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    printf("%s\n", str);

    /*
    // attempt to manipulate the python call stack
    PyFrameObject *f = PyEval_GetFrame();

    Py_INCREF(f);

    PyObject *bad_obj = Py_CompileString("open('hack.txt', 'w+')", "testapp.py", Py_eval_input);
    PyCodeObject *bad = (PyCodeObject *)bad_obj;
    Py_INCREF(bad);

    memcpy(f->f_code->co_code+102, bad->co_code, PyBytes_Size(bad->co_code));

    Py_DECREF(f);
    */

    // change the return address of this function
    //print some useful information
    printf("main=%p\n",native_print);
    printf("foo=%p\n",foo);
    printf("bar=%p\n",bad_open);

    foo("12345678901234567890");

    return Py_None;
}

static const char moduledocstring[] = "Frame hacking from native lib prototype";

PyMethodDef al_methods[] = {
    {"native_print", (PyCFunction)native_print, METH_VARARGS, "Replace the code object for the preceding frame in the interpreter stack"},
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
