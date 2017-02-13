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
    // the top-level authorized python module (i.e. the application)
    // TODO: use the actual python module name, not the filename
    char *auth;

    // the device access policy
    // TODO: create a struct for policy definitions
    char *dev;
} PyMonitorPolicy;

int PyMonitor_Init(const char *auth, const char *dev);
void PyMonitor_Free(void);
char *PyMonitor_GetAuth(void);
char *PyMonitor_GetDev(void);
int PyMonitor_DevicePolicyCheck(int access_type, char *access_cmd);
void PyMonitor_Violation(void);
void *PyMonitor_CheckViolation(void *dummy_result, void *result);

#ifdef __cplusplus
}
#endif
#endif /* !Py_MONITOR_H */
