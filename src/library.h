#include <stdio.h>
#define UPLOAD_DIR "/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/upload"
#define REPORTING_DIR "/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/reporting"
#define BACKUP_DIR "/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/backup"

#define TRANSFER_TIME "01:27"
#define BACKUP_TIME "01:28"

extern FILE *monitor_logfile;
extern FILE *daemon_logfile;

extern const char* DAEMON_NAME;
extern const char* PID_FILE_PATH;