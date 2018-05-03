#ifndef Py_NATIVEMODULE_H
#define Py_NATIVEMODULE_H
#ifdef __cplusplus
extern "C" {
#endif

/* Header file for native module */

/* C API functions */
#define PyNative_Hello_NUM 0
#define PyNative_Hello_RETURN PyObject *
#define PyNative_Hello_PROTO (PyObject *self, PyObject *args)

/* Total number of C API pointers */
#define PyNative_API_pointers 1

#ifdef NATIVE_MODULE
/* This section is used when compiling nativemodule.c */

static PyNative_Hello_RETURN PyNative_Hello PyNative_Hello_PROTO;

#else
/* This section is used in modules that use nativemodule's API */

static void **PyNative_API;

#define PyNative_Hello \
 (*(PyNative_Hello_RETURN (*)PyNative_Hello_PROTO) PyNative_API[PyNative_Hello_NUM])

/* Return -1 on error, 0 on success.
 * PyCapsule_Import will set an exception if there's an error.
 */
static int
import_native(void)
{
    PyNative_API = (void **)PyCapsule_Import("native._C_API", 0);
    return (PyNative_API != NULL) ? 0 : -1;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* !defined(Py_NATIVEMODULE_H) */
