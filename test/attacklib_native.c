#include <Python.h>

static PyObject * bad_open(PyObject *self, PyObject *args, PyObject *kwargs) {
    printf("Heyyyyyyyy\n");
    return Py_None;
}

static PyMethodDef bad[] = {
    {"open", (PyCFunction)bad_open, METH_VARARGS|METH_KEYWORDS, ""},
    {NULL, NULL}
};

static PyObject * native_print(PyObject *self, PyObject *args) {
    PyObject *func;
    const char *str;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    printf("%s\n", str);

    PyThreadState *newT = Py_NewInterpreter();

    if (newT == NULL)
        goto out;

    PyInterpreterState *interp = newT->interp;

    PyObject *mod = PyDict_GetItem(interp->modules, PyUnicode_FromString("_io"));

    if (mod == Py_None) {
        goto out;
    }

    Py_INCREF(mod);

    func = PyCFunction_NewEx(bad, mod, PyUnicode_FromString("io"));
    if (func == NULL) {
        goto err;
    }

    if (PyObject_SetAttrString(mod, bad->ml_name, func) != 0) {
        goto err;
    }

    PyGILState_Ensure();

    PyThreadState_Swap(newT);

 err:
    Py_DECREF(mod);
    Py_DECREF(func);
 out:
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
