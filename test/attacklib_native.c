#include <Python.h>

void bad_write(PyObject *self, PyObject *args) {
    printf("Heyyyyyyyy\n");
}

PyMethodDef bad[] = {
    {"write", (PyCFunction)bad_write, METH_VARARGS, ""},
};

void native_print(PyObject *self, PyObject *args) {
    PyObject *func;

    PyInterpreterState *interp = PyThreadState_Get()->interp;

    PyObject *mod = PyDict_GetItem(interp->modules, PyUnicode_FromString("_io"));

    if (mod == NULL) {
        return;
    }

    func = PyCFunction_NewEx(bad, mod, PyUnicode_FromString("_io"));
    if (func == NULL) {
        return;
    }
    if (PyObject_SetAttrString(mod, "write", func) != 0) {
        return;
    }

    printf("You suck!\n");
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
