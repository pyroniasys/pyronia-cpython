/* Implements the data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#include "Python.h"
#include "frameobject.h"
#include "dictobject.h"

typedef struct _cg_node {
    struct _cg_node *parent;

    // points to a linked list of children
    struct _cg_node *children;

    // points to a linked list of siblings
    struct _cg_node *siblings;

    //const char *filename;
    char *funcname;
} cgNode;

static PyMonitorPolicy *policy = NULL;
static PyObject *PolicyViolation;
static int is_build; // don't do monitoring if we're building python
// keep track of our current execution context: caller and callee functions
static PyObject *monitor_state = NULL;
static cgNode *callgraph = NULL;
static cgNode *cur_caller = NULL;

// forward declarations
static int tracer(PyObject *, PyFrameObject *, int, PyObject *);
static int cgNode_new(cgNode **, cgNode *, /*const char *,*/ const char *);
static void cgNode_dealloc(cgNode **);

int PyMonitor_Init(const char *auth, const char *d) {
    if (!strcmp(auth, "../setup.py") || !strcmp(auth, "sysconfig")) {
        is_build = 1;
        return 1;
    }

    is_build = 0;

    // fail if we already have a policy
    if (policy != NULL || callgraph != NULL) {
        // we don't want to lose the existing policy, so don't free
        return 0;
    }

    // init the context state
    monitor_state = PyDict_New();
    if (monitor_state == NULL) {
        PyErr_NoMemory();
        return 0;
    }

    if (cgNode_new(&callgraph, NULL, "root")) {
        goto err;
    }
    cur_caller = callgraph;

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

    // set the profiler
    PyEval_SetProfile(tracer, monitor_state);

    return 1;

 err:
    PyMonitor_Free();
    return 0;
}

void PyMonitor_Free() {
    PyEval_SetProfile(NULL, NULL);
    PyDict_Clear(monitor_state);
    Py_DECREF(monitor_state);
    cgNode_dealloc(&callgraph);
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
        goto out;

    if (policy->dev == NULL) {
        // TODO: set an error here
        goto out;
    }

    // this checks if the beginning of the access command matches the policy
    // if we're not in the authorized context, we have a problem
    // TODO: make sure pure string comp is sound
    if (!strncmp(policy->dev, access_cmd, strlen(policy->dev)) && strncmp("../test/monitordoorbell.py", policy->auth, strlen(policy->auth))) {
        //printf("[msm] %s:%s\n", _PyUnicode_AsString(caller_frame->f_code->co_filename), PyEval_GetFuncName(callee));
        PyMonitor_Violation();
        return 0;
    }

out:
    return 1;
}

static int cgNode_new(cgNode **new, cgNode *p, /*const char *file,*/ const char *func) {
    cgNode *node;

    node = (cgNode *)PyMem_RawMalloc(sizeof(cgNode));

    if (node == NULL) {
        PyErr_NoMemory();
        goto err;
    }

    node->parent = p;
    node->children = NULL;
    node->siblings = NULL;

    /*
    node->filename = PyMem_Malloc(strlen(file)+1);
    if (!node->filename) {
        PyErr_NoMemory();
        goto err;
    }

    memcpy(node->filename, file, strlen(file)+1);
    */

    node->funcname = PyMem_Malloc(strlen(func)+1);
    if (!node->funcname) {
        PyErr_NoMemory();
        goto err;
    }

    memcpy(node->funcname, func, strlen(func)+1);

    *new = node;
    return 0;
err:
    return 1;
}

static void cgNode_dealloc(cgNode **node) {
    cgNode *n;
    n = *node;

    if (n->children == NULL && n->siblings == NULL) {
        PyMem_Free(n->funcname);
        PyMem_RawFree(n);
        node = NULL;
        return;
    }
    else if (n->children != NULL && n->siblings == NULL) {
        cgNode_dealloc(&(n->children));
    }
    else if (n->children == NULL && n->siblings != NULL) {
        cgNode_dealloc(&(n->siblings));
    }
    else {
        cgNode_dealloc(&(n->children));
        cgNode_dealloc(&(n->siblings));
    }
}

static void cgNode_addChild(cgNode *node, cgNode *child) {
    // the return value of this function reflects the success of AddChild()
    if (node->children == NULL) {
        node->children = child;
        return;
    }

    cgNode *runner;

    runner = node->children;
    while (runner->siblings != NULL) {
        runner = runner->siblings;
    }

    runner->siblings = child;
}

static cgNode *cgNode_findChild(cgNode *node, const char *name) {
    cgNode *runner;

    runner = node->children;
    if (runner == NULL) {
        goto out;
    }

    while (runner->siblings != NULL) {
        if (!strcmp(runner->funcname, name))
            break;

        runner = runner->siblings;
    }

out:
    return runner;
}

static int tracer(PyObject *self, PyFrameObject *frame, int what, PyObject *arg) {
    PyCodeObject *caller_code, *callee_code;
    const char *caller_name, *callee_name;
    long depth;
    PyObject *last_callers, *last_callees;
    cgNode *cur_callee;

    depth = PyLong_AsLong(PyDict_GetItemString(self, "depth"));

    if (what == PyTrace_CALL || what == PyTrace_C_CALL) {
        // don't want our frame being gc'd on us
        Py_INCREF(frame);

        last_callers = PyDict_GetItemString(self, "last-callers");
        last_callees = PyDict_GetItemString(self, "last-callees");

        if (!PyDict_Check(last_callers) || !PyDict_Check(last_callees)) {
            PyErr_BadInternalCall();
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
                // reset caller node to the root of the call graph
                cur_caller = (cgNode *)callgraph;
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
            caller_name = "root";
        }

        if (PyDict_GetItem(last_callers, PyLong_FromLong(depth)) == NULL) {
            // this is the first time we enter this level
            PyDict_SetItem(last_callers, PyLong_FromLong(depth), PyUnicode_FromString(caller_name));
        }

        if (depth > 0 && !strcmp(_PyUnicode_AsString(PyDict_GetItem(last_callers, PyLong_FromLong(depth-1))), caller_name)) {
            caller_name = _PyUnicode_AsString(PyDict_GetItem(last_callees, PyLong_FromLong(depth-1)));
        }

        //printf("[msm] caller_name: %s, node_name: %s\n", caller_name, cur_caller->funcname);

        if (what == PyTrace_C_CALL) {
            callee_name = PyEval_GetFuncName(arg);
            printf("[msm] c_call - ");
        }
        else {
            callee_name = _PyUnicode_AsString(callee_code->co_name);
            printf("[msm] ");
        }

        cur_callee = cgNode_findChild(cur_caller, callee_name);
        if (cur_callee == NULL) {
            if (cgNode_new(&cur_callee, cur_caller, callee_name)) {
                return -1;
            }

            cgNode_addChild(cur_caller, cur_callee);
            printf("(new callee) ");
        }

        printf("%s --> %s\n", cur_caller->funcname, cur_callee->funcname);

        // finally, let's update our monitor state
        cur_caller = cur_callee;
        depth++;
        PyDict_SetItem(last_callers, PyLong_FromLong(depth), PyUnicode_FromString(caller_name));
        PyDict_SetItem(last_callees, PyLong_FromLong(depth), PyUnicode_FromString(callee_name));

        Py_DECREF(frame);

    }
    else if (what == PyTrace_RETURN || what == PyTrace_C_RETURN) {
        depth--;
        cur_caller = cur_caller->parent;
        printf("[msm] returning to %s\n", cur_caller->funcname);
    }

    // re-set the depth of the call graph (note: it might have not changed)
    PyDict_SetItemString(monitor_state, "depth", PyLong_FromLong(depth));

    return 0;

}
