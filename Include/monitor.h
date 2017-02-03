/* Interface to data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#ifndef Py_MONITOR_H
#define Py_MONITOR_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(int) PyMonitor_ExtProcCheck(PyObject *func, PyObject *args);

#ifdef __cplusplus
}
#endif
#endif /* !Py_MONITOR_H */
