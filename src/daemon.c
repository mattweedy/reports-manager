#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "library.h"

// TODO: Singleton processes
// 		- Daemon setup using an init script
// 		  implementing the singleton pattern
// 		  and using header files to store
// 		  configurable variables. The init
// 		  script can be used to start/stop the
// 		  daemon.


static void become_daemon(void);
void clean_exit(int sigid);

pid_t monitor_pid = -1;
pid_t file_transfer_pid = -1;

// Function to transform main process into a daemon
static void become_daemon(void)
{
	pid_t child_pid;

	// Step 1: Fork off the parent process
	child_pid = fork();

	// Error handling: exit if fork fails
	if (child_pid < 0)
	{
		exit(EXIT_FAILURE);
	}

	// Parent process exits if fork succeeds
	if (child_pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	// Step 2: Child process becomes session leader
	if (setsid() < 0)
	{
		exit(EXIT_FAILURE);
	}

	// Step 3: Ignore specific signals
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);


	// Step 4: Second fork to detach completely
	child_pid = fork();

	// Error handling: exit if fork fails
	if (child_pid < 0)
	{
		exit(EXIT_FAILURE);
	}

	// Parent process exits if fork succeeds
	if (child_pid > 0)
	{
		exit(EXIT_SUCCESS);
	}


	// Step 6: Change working directory
	// chdir("/");
	if (chdir("/mnt/d/Matthew/OneDrive - Technological University Dublin/Documents/year 4 sem 2/sys_soft/assignment/src") < 0)
	{
		syslog(LOG_ERR, "reports_manager : failed to change working directory to /");
		exit(EXIT_FAILURE);
	}

	// Step 7: Set new file permissions
	mode_t old_mask = umask(0);
	if (old_mask != 022)
	{
		syslog(LOG_WARNING, "reports_manager : unexpected umask: %03o", old_mask);
	}

	// Step 8: Close all open file descriptors
	for (long fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
	{
		close((int)fd);
	}

	// step 5: singleton pattern implementation
	int pid_file = open("/tmp/daemon.pid", O_CREAT | O_RDWR, 0666);
	int rc = flock(pid_file, LOCK_EX | LOCK_NB);
	syslog(LOG_NOTICE, "reports_manager : flock returned %d", rc);
	if (rc)
	{
		if (EWOULDBLOCK == errno)
		{
			syslog(LOG_ERR, "reports_manager : another instance is already running");
			exit(EXIT_FAILURE);
		}
	}

	// Open log file
	openlog("reports_manager", LOG_PID, LOG_DAEMON);
}



void clean_exit(int sigid) {
	if (sigid == SIGTERM || sigid == SIGINT) {
		syslog(LOG_NOTICE, "reports_manager terminated");
	}
	else {
		syslog(LOG_NOTICE, "Unidentified signal (%d) received", sigid);

	}
	if (monitor_pid != -1) {
		kill(monitor_pid, SIGTERM);
	}
	if (file_transfer_pid != -1) {
		kill(file_transfer_pid, SIGTERM);
	}
	closelog();
	exit(EXIT_SUCCESS);

}


int main(void)
{
	// char temp[1024];
	// char monitor_path[1024];
	// if (getcwd(temp, sizeof(temp)) != NULL) {
	// 	snprintf(monitor_path, sizeof(monitor_path), "%s/%s", temp, "monitor");
	// }
	// char file_transfer_path[1024];
	// if (getcwd(temp, sizeof(temp)) != NULL) {
	// 	snprintf(file_transfer_path, sizeof(file_transfer_path), "%s/%s", temp, "file_transfer");
	// }

	// transform into a daemon process
	become_daemon();

	signal(SIGINT, clean_exit);
	signal(SIGTERM, clean_exit);

	// fork monitor and file_transfer
	syslog(LOG_NOTICE, "reports_manager : starting monitor");
	monitor_pid = fork();
	if (monitor_pid == 0)
	{
		execl("monitor", "monitor", UPLOAD_DIR, NULL);
		syslog(LOG_ERR, "reports_manager : failed to start monitor : %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// do again for backup
	syslog(LOG_NOTICE, "reports_manager : starting file_transfer");
	file_transfer_pid = fork();
	if (file_transfer_pid == 0)
	{
		execl("file_transfer", "file_transfer", "-d", NULL);
		syslog(LOG_ERR, "reports_manager : failed to start file_transfer : %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// main loop: log every 20 seconds
	while (1)
	{
		syslog(LOG_NOTICE, "reports_manager running - PID [%d]", getpid());
		sleep(20);
	}

	// log termination and close log file
	syslog(LOG_NOTICE, "reports_manager terminated");
	closelog();

	return EXIT_SUCCESS;
}
