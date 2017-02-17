/* Implements the data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#include "Python.h"
#include "frameobject.h"
#include "dictobject.h"

static PyMonitorPolicy *policy;
static PyObject *PolicyViolation;
static int is_build; // don't do monitoring if we're building python
// keep track of our current execution context: caller and callee functions
static PyObject *monitor_state;

// forward declaration
static int tracer(PyObject *, PyFrameObject *, int, PyObject *);

int PyMonitor_Init(const char *auth, const char *d) {
    // fail if we already have a policy
    if (policy) {
        // we don't want to lose the existing policy, so don't free
        return 0;
    }

    // init the context state
    monitor_state = PyDict_New();
    if (monitor_state == NULL) {
        PyErr_NoMemory();
        return 0;
    }

    // let's start keeping track of the depth of our call graph
    PyDict_SetItemString(monitor_state, "depth", PyLong_FromLong(0));
    PyDict_SetItemString(monitor_state, "last-callers", PyDict_New());
    PyDict_SetItemString(monitor_state, "last-callees", PyDict_New());

    policy = (PyMonitorPolicy *)PyMem_RawMalloc(sizeof(PyMonitorPolicy));

    if (policy == NULL) {
        PyErr_NoMemory();
        goto err;
    }

    policy->auth = PyMem_Malloc(strlen(auth)+1);
    if (!policy->auth) {
        PyErr_NoMemory();
        goto err;
    }

    memcpy(policy->auth, auth, strlen(auth)+1);

    policy->dev = PyMem_Malloc(strlen(d)+1);
    if (!policy->dev) {
        PyErr_NoMemory();
        goto err;
    }

    memcpy(policy->dev, d, strlen(d)+1);

    printf("[msm] policy->auth = %s\n", policy->auth);

    // let's check if this is being called as part of our python build
    if (!strcmp(policy->auth, "../setup.py") || !strcmp(policy->auth, "sysconfig")) {
        is_build = 1;
    }
    else {
        is_build = 0;
        // set the profiler
        PyEval_SetProfile(tracer, monitor_state);
    }

    return 1;

 err:
    PyMonitor_Free();
    return 0;
}

void PyMonitor_Free() {
    PyEval_SetProfile(NULL, NULL);
    PyMem_Free(policy->auth);
    PyMem_Free(policy->dev);
    PyMem_RawFree(policy);
}

char *PyMonitor_GetAuth() {
    return policy->auth;
}

char *PyMonitor_GetDev() {
    return policy->dev;
}

void PyMonitor_Violation() {
    PolicyViolation = PyErr_NewException("monitor.violation", NULL, NULL);
    Py_INCREF(PolicyViolation);
    PyErr_SetString(PolicyViolation, "Unauthorized access to device");
}

/* Checks whether a device policy violation was raised and returns a dummy
 * result if it was, and returns the actual result set by the function
 * otherwise. If a policy violation is raised, the error number is
 * cleared to avoid propagating the error to the interpreter, which in
 * turn allows the main program to continue executing.
 */
void *PyMonitor_CheckViolation(void *dummy_result, void *result) {
    if (PyErr_ExceptionMatches(PolicyViolation)) {
        PyErr_Clear();
        return dummy_result;
    }

    return result;
}

int PyMonitor_DevicePolicyCheck(int access_type, char *access_cmd) {
<<<<<<< 5e5df33de2759d5fbdbd9e2108e283780bd25fde
    // don't actually monitor python during the build process
    if (is_build)
        return 1;
=======
    char *cur_callee_filename;

    // don't actually monitor python during the build process
    if (is_build || monitor_state == NULL)
        goto out;
>>>>>>> Port tracer from iot-app-analysis/tracer

    if (policy->dev == NULL) {
        // TODO: set an error here
        goto out;
    }

<<<<<<< 5e5df33de2759d5fbdbd9e2108e283780bd25fde
    // this checks if the beginning of the access command matches
    // TODO: make sure this is sound
    if (!strncmp(policy->dev, access_cmd, strlen(policy->dev))) {
      return 1;
    }

    printf("[msm] policy: %s, bad access cmd: %s\n", policy->dev, access_cmd);
    PyMonitor_Violation();

 out:
=======
    // this checks if the beginning of the access command matches the policy
    // if we're not in the authorized context, we have a problem
    // TODO: make sure pure string comp is sound
    cur_callee_filename = _PyUnicode_AsString(PyDict_GetItemString(monitor_state, "cur-callee-filename"));
    if (!strncmp(policy->dev, access_cmd, strlen(policy->dev)) && strncmp(cur_callee_filename, policy->auth, strlen(policy->auth))) {
        //printf("[msm] %s:%s\n", _PyUnicode_AsString(caller_frame->f_code->co_filename), PyEval_GetFuncName(callee));
        PyMonitor_Violation();
        return 0;
    }

out:
    return 1;
}

static int tracer(PyObject *self, PyFrameObject *frame, int what, PyObject *arg) {
    PyCodeObject *caller_code, *callee_code;
    const char *caller_name, *callee_name;
    long depth;
    PyObject *last_callers, *last_callees;

    if (!PyDict_Check(self)) {
        return -1;
    }
    depth = PyLong_AsLong(PyDict_GetItemString(self, "depth"));

    if (what == PyTrace_CALL || what == PyTrace_C_CALL) {
        // don't want our frame being gc'd on us
        Py_INCREF(frame);

        last_callers = PyDict_GetItemString(self, "last-callers");
        last_callees = PyDict_GetItemString(self, "last-callees");

        if (!PyDict_Check(last_callers) || !PyDict_Check(last_callees)) {
            return -1;
        }

        callee_code = frame->f_code;
        if (frame->f_back != NULL) {
            caller_code = frame->f_back->f_code;
        }
        else {
            caller_code = NULL;
            if (depth > 0){
                printf("[msm] merp\n");
                depth = 0;
            }
        }

        // if we're in a c call, the caller of the c function
        // is actually the callee of the last python frame
        if (what == PyTrace_C_CALL) {
            caller_code = callee_code;
        }

        if (caller_code != NULL) {
            caller_name = _PyUnicode_AsString(caller_code->co_name);
        }
        else {
            caller_name = "null";
        }

        if (PyDict_GetItem(last_callers, PyLong_FromLong(depth)) == NULL) {
            // this is the first time we enter this level
            PyDict_SetItem(last_callers, PyLong_FromLong(depth), PyUnicode_FromString(caller_name));
        }

        if (depth > 0 && !strcmp(_PyUnicode_AsString(PyDict_GetItem(last_callers, PyLong_FromLong(depth-1))), caller_name)) {
            caller_name = _PyUnicode_AsString(PyDict_GetItem(last_callees, PyLong_FromLong(depth-1)));
        }

        if (what == PyTrace_C_CALL) {
            callee_name = PyEval_GetFuncName(arg);
            printf("[msm] c_call: %s --> %s\n", caller_name, callee_name);
        }
        else {
            callee_name = _PyUnicode_AsString(callee_code->co_name);
            printf("[msm] %s --> %s\n", caller_name, callee_name);
        }

        // finally, let's update our monitor state
        PyDict_SetItemString(self, "cur-caller", PyUnicode_FromString(caller_name));
        PyDict_SetItemString(self, "cur-callee", PyUnicode_FromString(callee_name));
        PyDict_SetItemString(self, "cur-callee-filename", callee_code->co_filename);

        depth++;
        PyDict_SetItem(last_callers, PyLong_FromLong(depth), PyUnicode_FromString(caller_name));
        PyDict_SetItem(last_callees, PyLong_FromLong(depth), PyUnicode_FromString(callee_name));

        Py_DECREF(frame);

    }
    else if (what == PyTrace_RETURN || what == PyTrace_C_RETURN) {
        depth--;
    }

    // re-set the depth of the call graph (note: it might have not changed)
    PyDict_SetItemString(monitor_state, "depth", PyLong_FromLong(depth));

>>>>>>> Port tracer from iot-app-analysis/tracer
    return 0;

}
