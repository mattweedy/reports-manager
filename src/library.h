#include <stdio.h>
#define UPLOAD_DIR "/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/upload"
#define REPORTING_DIR "/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/reporting"
#define BACKUP_DIR "/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/backup"

#define MUST_UPLOAD_BY_TIME "23:30"
#define TRANSFER_TIME "01:00"
#define BACKUP_TIME "01:30"

extern FILE *monitor_logfile;

extern const char* DAEMON_NAME;
extern const char* PID_FILE_PATH;