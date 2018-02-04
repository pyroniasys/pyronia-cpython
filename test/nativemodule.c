#include <Python.h>
#include <stdio.h>

#define NATIVE_MODULE
#include "nativemodule.h"

// Prints hello
static PyObject * PyNative_Hello(PyObject *self, PyObject *args) {
    printf("hello\n");

    Py_INCREF(Py_None);
    return Py_None;
}

static void print_wrapper(void) {
    printf("hidden hello\n");
}

// calls an internal function to print hello
static PyObject * hello_hidden(PyObject *self, PyObject *args) {
    print_wrapper();

    Py_INCREF(Py_None);
    return Py_None;
}

// calls the exported function to print hello
static PyObject * hello_wrapped(PyObject *self, PyObject *args) {
    return PyNative_Hello(self, args);
}

PyMethodDef test_methods[] = {
    {"hello", (PyCFunction)PyNative_Hello, METH_NOARGS, "Printf hello"},
    {"hidden_hello", (PyCFunction)hello_hidden, METH_NOARGS, "Hidden printf hello"},
    {"wrapped_hello", (PyCFunction)hello_wrapped, METH_NOARGS, "Wrapped printf hello"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initnative(void)
{
    PyObject *m;
    static void *PyNative_API[PyNative_API_pointers];
    PyObject *c_api_object;

    m = Py_InitModule("native", test_methods);
    if (m == NULL)
        return;

    /* Initialize the C API pointer array */
    PyNative_API[PyNative_Hello_NUM] = (void *)PyNative_Hello;

    /* Create a Capsule containing the API pointer array's address */
    c_api_object = PyCapsule_New((void *)PyNative_API, "native._C_API", NULL);

    if (c_api_object != NULL)
        PyModule_AddObject(m, "_C_API", c_api_object);
}
