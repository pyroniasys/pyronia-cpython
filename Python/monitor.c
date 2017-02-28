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

    char *filename;
    char *funcname;

    // TODO: add module and args (and env vars?!)
} cgNode;

typedef struct _auth_ctx {
    cgNode *exec_ctx;
    // TODO: add access token and/or taint label
} authCtx;

typedef struct _monitor_policy {
    // the authorized python module (i.e. the application)
    authCtx *auth_ctx;

    // the device access policy
    // TODO: create a struct for policy definitions
    char *auth;
    char *auth_func;
    char *access_file;
    char *access_func;
    // TODO: incorporate this into the ctx
    char *dev;
} PyMonitorPolicy;

static PyMonitorPolicy *policy = NULL;
static PyObject *PolicyViolation;
static int is_build; // don't do monitoring if we're building python
static int is_tracing = 0;
// keep track of our current execution context: caller and callee functions
static PyObject *monitor_state = NULL;
static cgNode *callgraph = NULL;
static cgNode *cur_caller = NULL;

// forward declarations
static int tracer(PyObject *, PyFrameObject *, int, PyObject *);
static int cgNode_new(cgNode **, cgNode *, const char *, const char *);
static void cgNode_free(cgNode **);
static int authCtx_new(authCtx **);
static void authCtx_free(authCtx **);

static int alloc_str(char **new_str, const char *to_save) {
    char *str;
    size_t len;

    len = strlen(to_save)+1;
    str = PyMem_Calloc(1, len);
    if (!str) {
        PyErr_NoMemory();
        return -1;
    }

    memcpy(str, to_save, len-1);

    *new_str = str;
    return 0;
}

static int policy_init(PyMonitorPolicy **new, const char *auth, const char *auth_func, const char *access_file, const char *access_func, const char *d) {
    PyMonitorPolicy *policy;
    policy = (PyMonitorPolicy *)PyMem_RawCalloc(1, sizeof(PyMonitorPolicy));

    if (policy == NULL) {
        PyErr_NoMemory();
        goto err;
    }

    if (alloc_str(&(policy->auth), auth) ||
        alloc_str(&(policy->auth_func), auth_func) ||
        alloc_str(&(policy->access_file), access_file) ||
        alloc_str(&(policy->access_func), access_func) ||
        alloc_str(&(policy->dev), d)) {
        // error set by alloc_str
        goto err;
    }

    if (authCtx_new(&(policy->auth_ctx))) {
        // error set by authCtx_new
        goto err;
    }

    *new = policy;
    return 0;
 err:
    // PyMonitor_Init will free the policy
    return -1;
}

int PyMonitor_Init(const char *auth, const char *auth_func, const char *access_file, const char *access_func, const char *d) {
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

    // let's initialize our main monitor policy
    if (policy_init(&policy, auth, auth_func, access_file, access_func, d)) {
        // error set by policy_init
        goto err;
    }

    if (cgNode_new(&callgraph, NULL, auth, "root")) {
        goto err;
    }
    cur_caller = callgraph;

    // let's start keeping track of the depth of our call graph
    PyDict_SetItemString(monitor_state, "depth", PyLong_FromLong(0));

    // set the profiler
    PyEval_SetProfile(tracer, monitor_state);
    is_tracing = 1;

    return 1;

 err:
    PyMonitor_Free();
    return 0;
}

void PyMonitor_Free() {
    PyEval_SetProfile(NULL, NULL);
    is_tracing = 0;
    PyDict_Clear(monitor_state);
    Py_DECREF(monitor_state);
    cgNode_free(&callgraph);
    // TODO: clean this up a bit: create policy_free
    authCtx_free(&(policy->auth_ctx));
    PyMem_Free(policy->auth);
    PyMem_Free(policy->auth_func);
    PyMem_Free(policy->access_file);
    PyMem_Free(policy->access_func);
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
        return 0;
    }

    // this checks if the beginning of the access command matches the policy
    // if we're not in the authorized context, we have a problem
    // TODO: ensure that cur_caller->parent is ALWAYS the right node to look at
    if (!strncmp(policy->dev, access_cmd, strlen(policy->dev)) && memcmp(cur_caller->parent, policy->auth_ctx->exec_ctx, sizeof(cgNode))) {
        PyMonitor_Violation();
        return 0;
    }

out:
    return 1;
}

static int authCtx_new(authCtx **new) {
    authCtx *c;

    c = (authCtx *)PyMem_RawCalloc(1, sizeof(authCtx));

    if (c == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    // this is set by PyMonitor_CheckSetAuthExecCtx
    c->exec_ctx = NULL;

    *new = c;
    return 0;
}

static void authCtx_free(authCtx **ctx) {
    authCtx *c;

    if (ctx == NULL)
        return;

    c = *ctx;

    if (c->exec_ctx != NULL) {
        cgNode_free(&(c->exec_ctx));
    }
    PyMem_RawFree(c);
    ctx = NULL;
}

static int cgNode_new(cgNode **new, cgNode *p, const char *file, const char *func) {
    cgNode *node;

    node = (cgNode *)PyMem_RawCalloc(1, sizeof(cgNode));

    if (node == NULL) {
        PyErr_NoMemory();
        goto err;
    }

    node->parent = p;
    node->children = NULL;
    node->siblings = NULL;

     if (alloc_str(&(node->filename), file) ||
        alloc_str(&(node->funcname), func)) {
        // error set by alloc_str
        goto err;
    }

    *new = node;
    return 0;
err:
    return -1;
}

static void cgNode_free(cgNode **node) {
    cgNode *n;
    n = *node;

    if (n->children == NULL && n->siblings == NULL) {
        PyMem_Free(n->funcname);
        PyMem_Free(n->filename);
        PyMem_RawFree(n);
        node = NULL;
        return;
    }
    else if (n->children != NULL && n->siblings == NULL) {
        cgNode_free(&(n->children));
    }
    else if (n->children == NULL && n->siblings != NULL) {
        cgNode_free(&(n->siblings));
    }
    else {
        cgNode_free(&(n->children));
        cgNode_free(&(n->siblings));
    }
}

static void cgNode_addChild(cgNode *node, cgNode *child) {
    cgNode *runner;

    // the return value of this function reflects the success of AddChild()
    if (node->children == NULL) {
        node->children = child;
        return;
    }

    runner = node->children;
    while (runner->siblings != NULL) {
        runner = runner->siblings;
    }

    runner->siblings = child;
}

static cgNode *cgNode_findChild(cgNode *node, const char *fname, const char *name) {
    cgNode *runner;

    runner = node->children;
    if (runner == NULL) {
        goto out;
    }

    while (runner != NULL) {
        if (!strcmp(runner->funcname, name) && !strcmp(runner->filename, fname))
            break;

        runner = runner->siblings;
    }

 out:
    return runner;
}

void PyMonitor_CheckSetAuthExecCtx(PyObject *func) {

    // we'll enter here while python is setting up but we
    // haven't begun executing __main__ yet (which is when we init the monitor)
    // we also don't want to reset the authoritized exec context
    // if we've already done so
    if (!is_tracing || policy->auth_ctx->exec_ctx != NULL)
        return;

    Py_INCREF(func);

    if (!strcmp(PyEval_GetFuncName(func), policy->access_func) &&
        !strcmp(PyEval_GetFileName(func), policy->access_file) &&
        !strcmp(cur_caller->filename, policy->auth) &&
        !strcmp(cur_caller->funcname, policy->auth_func)) {
        policy->auth_ctx->exec_ctx = cur_caller;
    }

    Py_DECREF(func);
}

static int tracer(PyObject *self, PyFrameObject *frame, int what, PyObject *arg) {
    PyCodeObject *callee_code;
    const char *callee_name, *callee_fname;
    long depth;
    cgNode *cur_callee;

    depth = PyLong_AsLong(PyDict_GetItemString(self, "depth"));

    if (what == PyTrace_CALL || what == PyTrace_C_CALL) {
        // don't want our frame being gc'd on us
        Py_INCREF(frame);

        callee_code = frame->f_code;
        if (frame->f_back == NULL) {
            if (depth > 0){
                depth = 0;
                // reset caller node to the root of the call graph
                cur_caller = cgNode_findChild(callgraph, policy->auth, "<module>");
            }
        }

        if (what == PyTrace_C_CALL) {
            callee_name = PyEval_GetFuncName(arg);
            callee_fname = PyEval_GetFileName(arg);
            //printf("[msm] c_call - ");
        }
        else {
            callee_name = _PyUnicode_AsString(callee_code->co_name);
            callee_fname = _PyUnicode_AsString(callee_code->co_filename);
            //printf("[msm] ");
        }

        cur_callee = cgNode_findChild(cur_caller, callee_fname, callee_name);
        if (cur_callee == NULL) {
          if (cgNode_new(&cur_callee, cur_caller, callee_fname, callee_name)) {
                return -1;
            }

            cgNode_addChild(cur_caller, cur_callee);
        }

        //printf("%s:%s --> %s:%s\n", cur_caller->filename, cur_caller->funcname, callee_fname, callee_name);

        // finally, let's update our monitor state
        cur_caller = cur_callee;
        depth++;

        Py_DECREF(frame);

    }
    else if (what == PyTrace_RETURN || what == PyTrace_C_RETURN) {
        depth--;
        cur_caller = cur_caller->parent;
        //printf("[msm] returning to %s:%s\n", cur_caller->filename, cur_caller->funcname);
    }

    // re-set the depth of the call graph (note: it might have not changed)
    PyDict_SetItemString(monitor_state, "depth", PyLong_FromLong(depth));

    return 0;

}
