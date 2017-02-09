/* Implements the data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#include "Python.h"

static PyMonitorPolicy *policy;
static PyObject * PolicyViolation;

int PyMonitor_Init(const char *auth, const char *d) {
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

    return 1;

 err:
    PyMonitor_Free();
    return 0;
}

void PyMonitor_Free() {
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

int PyMonitor_IsViolation() {
    if (PyErr_ExceptionMatches(PolicyViolation)) {
        return 1;
    }
    return 0;
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
