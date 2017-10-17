#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int global_var = 12;

// Pass an immutable Python object and try to modify it's value
static PyObject * pass_immutable(PyObject *self, PyObject *args) {
    int value;

    if (!PyArg_ParseTuple(args, "i", &value))
        return NULL;

    value = 4;

    Py_INCREF(Py_None);
    return Py_None;
}

static const char moduledocstring[] = "Memory interactions testing from native lib prototype";

PyMethodDef memtest_methods[] = {
    {"pass_by_value", (PyCFunction)pass_immutable, METH_VARARGS, "Tries to change the value of an immutable object"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef memtestmodule = {
   PyModuleDef_HEAD_INIT,
   "memtestlib_native",      // name of module
   moduledocstring,  // module documentation, may be NULL
   -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
   memtest_methods
};

PyMODINIT_FUNC
PyInit_memtestlib_native(void) {
    PyObject *pInt = Py_BuildValue("i", global_var);
    PyObject *mod = PyModule_Create(&memtestmodule);
    PyObject_SetAttrString(mod, "global_var", pInt);

    return mod;
}
