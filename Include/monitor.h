/* Interface to data leak monitor.
 *
 * Author: Marcela S. Melara
 */

#ifndef Py_MONITOR_H
#define Py_MONITOR_H
#ifdef __cplusplus
extern "C" {
#endif

int PyMonitor_Init(const char *auth, const char *auth_func, const char *access_file, const char *access_func, const char *d);
void PyMonitor_Free(void);
char *PyMonitor_GetAuth(void);
char *PyMonitor_GetDev(void);
int PyMonitor_DevicePolicyCheck(int access_type, char *access_cmd);
void PyMonitor_CheckSetAuthExecCtx(PyObject *func);
void PyMonitor_Violation(void);
void *PyMonitor_CheckViolation(void *dummy_result, void *result);

#ifdef __cplusplus
}
#endif
#endif /* !Py_MONITOR_H */
