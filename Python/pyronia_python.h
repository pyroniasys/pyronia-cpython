/** Defines the Python-specific Pyronia functionality: Callstack generator
 * and module isolation. This functionality is passed into libpyronia
 * as callback functions.
 *
 *@author: Marcela S. Melara
 */
#ifndef __PY_PYRONIA_H
#define __PY_PYRONIA_H

#include <pyronia_lib.h>

#define LIB_POLICY "/home/pyronia/cpython/profiles/home.pyronia.cpython.pyronia_build.python-lib.prof"

#define PYR_LOGGING 0
#define pyrlog(format, ...) { \
    if (PYR_LOGGING) { \
      fprintf(stdout, "[pyronia] " format, ##__VA_ARGS__); \
      fflush(NULL);   \
    }\
  }

#ifdef Py_PYRONIA_BENCH
int max_dep_depth;
#endif

struct _ts;
typedef struct _ts PyThreadState; // forward declarations
PyThreadState *pyr_interp_tstate;

void acquire_gil(void);
void release_gil(void);

PyObject *PyDict_ProtectedNew(void);
void PyDict_ProtectedWrite(register PyObject *, int);
PyObject *_PyObject_GC_SecureMalloc(size_t);
PyObject *_PyObject_GC_NewSecure(PyTypeObject *tp);
PyVarObject *_PyObject_GC_NewSecureVar(PyTypeObject *, Py_ssize_t);
#define PyObject_GC_NewSecure(type, typeobj) \
                ( (type *) _PyObject_GC_NewSecure((typeobj)) )
#define PyObject_GC_NewSecureVar(type, typeobj, n) \
                ( (type *) _PyObject_GC_NewSecureVar((typeobj), (n)) )
void PyObject_GC_SecureDel(void *);

// these are wrappers around interp dom write access grant and revokes
// to enable toggling pyronia on and off (and so we don't need to import pyronia_lib.h directly anywhere but here)
#ifdef Py_PYRONIA
#define critical_state_alloc_pre(op)  \
  ( pyr_grant_critical_state_write((void *)op) )
#else
#define critical_state_alloc_pre(op) \
    do { } while(0)
#endif

#ifdef Py_PYRONIA
#define critical_state_alloc_post(op) \
  ( pyr_revoke_critical_state_write((void *)op) )
#else
#define critical_state_alloc_post(op) \
    do { } while(0)
#endif

#define Py_GetFullFuncName(func_fqn, mod, func)				\
  snprintf(func_fqn, strlen(func)+strlen(mod)+2, "%s.%s", mod, func)

int Py_Generate_Pyronia_Callstack(void);

#endif /* __PY_PYRONIA_H */
