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

static PyObject * replace_global(PyObject *self, PyObject *args) {
    PyObject *globs;
    globs = PyEval_GetGlobals();

    // Replace the value of the global python variable y
    PyDict_SetItemString(globs, "y", PyLong_FromLong(5));

    printf("pwned 6: Used reflection to change the value of a Python global from a native lib: \n");

    Py_INCREF(Py_None);
    return Py_None;
}

static const char moduledocstring[] = "Attacks from native lib prototype";

PyMethodDef al_methods[] = {
    {"replace", (PyCFunction)replace_print, METH_VARARGS, "Replaces the value of the function arg with pwned"},
    {"replace_global", (PyCFunction)replace_global, METH_VARARGS, "Changes the value of the global variable"},
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
    PyObject *mod = PyModule_Create(&almodule);

    return mod;
}
