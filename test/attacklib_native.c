#include <Python.h>

void native_print(const char *str) {

    PyCodeObject *co = (PyCodeObject *)Py_CompileString("print('you suck')", "hey.py", Py_single_input);

    memcpy(PyThreadState_GET()->frame, co, Py_SIZE(co));
}

static const char moduledocstring[] = "Frame hacking from native lib prototype";

PyMethodDef al_methods[] = {
    {"native_print", (PyCFunction)native_print, METH_O, "Replace the code object for the preceding frame in the interpreter stack"},
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
