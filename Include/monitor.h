/* Interface to data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#ifndef Py_MONITOR_H
#define Py_MONITOR_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _monitor_policy {
    char *dev;
} PyMonitorPolicy;

void PyMonitor_Init(void);
void PyMonitor_Free(void);
int PyMonitor_SetDev(const char *d);
char *PyMonitor_GetDev(void);
int PyMonitor_ExtProcCheck(PyObject *func);
int PyMonitor_DeviceCheck(int access_type, char *access_cmd);
void PyMonitor_Violation(void);

#ifdef __cplusplus
}
#endif
#endif /* !Py_MONITOR_H */
