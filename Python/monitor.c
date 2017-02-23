/* Implements the data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#include "Python.h"

static PyMonitorPolicy *policy;
static PyObject *PolicyViolation;
static int is_build; // don't do monitoring if we're building python

int PyMonitor_Init(const char *auth, const char *d) {
    // fail if we already have a policy
    if (policy) {
        // we don't want to lose the existing policy, so don't free
        return 0;
    }

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
    }

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
    // don't actually monitor python during the build process
    if (is_build)
        return 1;

    if (policy->dev == NULL) {
        // TODO: set an error here
        goto out;
    }

    // this checks if the beginning of the access command matches
    // TODO: make sure this is sound
    if (!strncmp(policy->dev, access_cmd, strlen(policy->dev))) {
      return 1;
    }

    PyMonitor_Violation();

 out:
    return 0;
}
