#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
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

	// Step 5: Change working directory
	chdir("/");

	// Step 6: Set new file permissions
	umask(0);

	// Step 7: Close all open file descriptors
	for (long fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
	{
		close((int)fd);
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
	printf("reports_manager : running as [%d]\n", getpid());
	// transform into a daemon process
	become_daemon();

	signal(SIGINT, clean_exit);
	signal(SIGTERM, clean_exit);

	// here code
	monitor_pid = fork();
	if (monitor_pid == 0)
	{
		execl("monitor", "monitor", "%s", UPLOAD_DIR, NULL); // adjust paths
		exit(EXIT_FAILURE);
	}
	// do again for backup
	file_transfer_pid = fork();
	if (file_transfer_pid == 0)
	{
		execl("file_transfer", "file_transfer", "-d", NULL);
		printf("reports_manager : file_transfer daemon failed to start\n");
		exit(EXIT_FAILURE);
	}

	// Main loop: log every 20 seconds
	while (1)
	{
		syslog(LOG_NOTICE, "reports_manager started");
		sleep(20);
		// break; - will cause pain
	}

	// Log termination and close log file
	syslog(LOG_NOTICE, "reports_manager terminated");
	closelog();

	return EXIT_SUCCESS;
}
