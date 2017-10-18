#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>

int global_var = 12;

static PyObject *my_callback = NULL;
static jmp_buf buf;
static const char *STAR = "*";
static const char *DOLLAR = "$";

// Pass an immutable Python object and try to modify it's value
static PyObject * pass_immutable(PyObject *self, PyObject *args) {
    int value;

    if (!PyArg_ParseTuple(args, "i", &value))
        return NULL;

    value = 5;

    Py_INCREF(Py_None);
    return Py_None;
}

// Pass a mutable Python object and modify it's value
static PyObject * pass_mutable(PyObject *self, PyObject *args) {
    PyObject *list;

    if (!PyArg_ParseTuple(args, "O", &list))
        return NULL;

    if (!PyList_Check(list))
        return NULL;

    if(PyList_SetItem(list, 0, PyLong_FromLong(6)))
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

// Set the callback into a Python function
static PyObject * set_callback(PyObject *self, PyObject *args) {
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(my_callback);  /* Dispose of previous callback */
        my_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

// Make the callback
static PyObject * make_callback(PyObject *self, PyObject *args) {
    int arg;
    PyObject *arglist;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "i", &arg))
        return NULL;

    /* Time to call the callback */
    arglist = Py_BuildValue("(i)", arg);
    result = PyObject_CallObject(my_callback, arglist);
    Py_DECREF(arglist);
    Py_DECREF(result);

    Py_INCREF(Py_None);
    return Py_None;
}

// Try to make a longjmp back into main
static void star(void) {
    printf("Native jumping back to Python main\n");
    longjmp(buf, 1);
}

static void dollar(const char *str) {
    star();
    printf("%s%s%s\n", DOLLAR, str, DOLLAR);
}

// Does a long jump back into the main Python program
// goes back via a callback
static PyObject * pretty_print(PyObject *self, PyObject *args) {
    const char *str;
    char *pretty;
    PyObject *obj;
    PyObject *arglist;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "sO", &str, &obj))
        return NULL;

    if (!setjmp(buf)) {
        dollar(str);
    }
    else {
        // call back into the main Python program
        if (!PyCallable_Check(obj)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(obj);

        asprintf(&pretty, "%s%s%s", STAR, str, STAR);
        arglist = Py_BuildValue("(s)", pretty);
        result = PyObject_CallObject(obj, arglist);
        Py_DECREF(result);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static const char moduledocstring[] = "Memory interactions testing from native lib prototype";

PyMethodDef memtest_methods[] = {
    {"pass_by_value", (PyCFunction)pass_immutable, METH_VARARGS, "Tries to change the value of an immutable object"},
    {"pass_by_ref", (PyCFunction)pass_mutable, METH_VARARGS, "Tries to change the value of a mutable object"},
    {"set_callback", (PyCFunction)set_callback, METH_VARARGS, "Sets a Python callback function to later be called by this native lib"},
    {"make_callback", (PyCFunction)make_callback, METH_VARARGS, "Calls back into a Python function"},
    {"pretty_print", (PyCFunction)pretty_print, METH_VARARGS, "Does a long jump and calls back into the Python main"},
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
