/* Implements the data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#include "Python.h"

// check if the function name and module match any of the expected
// methods for exec'ing an external process
static int is_ext_proc_call(PyObject *func) {
    // os.system (in posix module)
    if (!strcmp(PyEval_GetFuncName(func), "system") && !strcmp(PyEval_GetModuleName(func), "posix")) {
        return 1;
    }
    // this case covers subprocess.Popen
    else if (!strcmp(PyEval_GetFuncName(func), "fork_exec") && !strcmp(PyEval_GetModuleName(func), "_posixsubprocess")) {
        return 1;
    }
    return 0;
}

int PyMonitor_ExtProcCheck(PyObject *func, PyObject *args) {
    int is_ext = 0;

    // check if we're making a call to an external proc
    if (is_ext_proc_call(func)) {
        Py_ssize_t size;
        PyObject *cmd;
        // need to get the flag type to know how to parse
        // the arguments
        int flags = PyCFunction_GET_FLAGS(func) & ~(METH_CLASS | METH_STATIC | METH_COEXIST);

        switch(flags) {
        case METH_VARARGS:
            // TODO: handle this case
            printf("[msm] varargs\n");
            break;
        case METH_O:
            // get the command from the arguments
            size = PyTuple_GET_SIZE(args);
            if (size != 1) {
                // TODO: debug this now
                return 0;
            }

            cmd = PyTuple_GET_ITEM(args, 0);
            printf("[msm] command: %s\n", _PyUnicode_AsString(cmd));
            break;
        default:
            break;
        }
        is_ext = 1;
    }

    return is_ext;
}
