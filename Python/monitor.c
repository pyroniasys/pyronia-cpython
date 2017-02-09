/* Implements the data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#include "Python.h"

static PyMonitorPolicy *policy;
static PyObject * PolicyViolation;

// check if the function name and module match any of the expected
// methods for exec'ing an external process
static int is_ext_proc_call(PyObject *func) {
    // os.system (in posix module)
    if (!strcmp(PyEval_GetFuncName(func), "system") && !strcmp(PyEval_GetModuleName(func), "posix")) {
        return 1;
    }
    // this case covers subprocess.Popen
    else if (!strcmp(PyEval_GetFuncName(func), "_exec_child") && !strcmp(PyEval_GetModuleName(func), "subprocess")) {
        return 1;
    }
    return 0;
}

void PyMonitor_Init() {
    policy = (PyMonitorPolicy *)PyMem_RawMalloc(sizeof(PyMonitorPolicy));

    if (policy != NULL) {
        policy->dev = NULL;
    }
}

void PyMonitor_Free() {
    PyMem_Free(policy->dev);
    PyMem_RawFree(policy);
}

int PyMonitor_SetDev(const char* d) {
    policy->dev = PyMem_Malloc(strlen(d)+1);
    if (!policy->dev) {
        PyErr_NoMemory();
        return 0;
    }

    memcpy(policy->dev, d, strlen(d)+1);
    return 1;
}

char *PyMonitor_GetDev() {
    return policy->dev;
}

void PyMonitor_Violation() {
    PolicyViolation = PyErr_NewException("monitor.violation", NULL, NULL);
    Py_INCREF(PolicyViolation);
    PyErr_SetString(PolicyViolation, "Unauthorized access to device");
}

PyObject *PyMonitor_GetViolation() {
    return PolicyViolation;
}

int PyMonitor_ExtProcCheck(PyObject *func) {
    int is_ext = 0;

    // check if we're making a call to an external proc
    if (is_ext_proc_call(func)) {
        Py_ssize_t size;
        PyObject *cmd;
        /*
        // need to get the flag type to know how to parse
        // the arguments
        int flags = PyCFunction_GET_FLAGS(func) & ~(METH_CLASS | METH_STATIC | METH_COEXIST | METH_KEYWORDS);

        switch(flags) {
        case METH_VARARGS:
            // TODO: handle this case
            // get the command from the arguments
            size = PyTuple_GET_SIZE(args);
            if (size >= 1) {
                if(!PyArg_UnpackTuple(args, "cmd", 1, 1, &cmd)) {
                    return 0;
                }
            }
            else{
                return 0;
            }

            break;
        case METH_O:
            // get the command from the arguments
            size = PyTuple_GET_SIZE(args);
            if (size != 1) {
                return 0;
            }

            cmd = PyTuple_GET_ITEM(args, 0);
            break;
        default:
                return 0;
        }
        */
        is_ext = 1;
        //printf("[msm] command: %s\n", _PyUnicode_AsString(cmd));
    }

    return is_ext;
}

int PyMonitor_DeviceCheck(int access_type, char *access_cmd) {
    if (policy->dev == NULL) {
        return 0;
    }

    // this checks if the beginning of the access command matches
    // TODO: make sure this is sound
    if (!strncmp(policy->dev, access_cmd, strlen(policy->dev))) {
        return 1;
    }

    PyMonitor_Violation();
    return 0;
}
