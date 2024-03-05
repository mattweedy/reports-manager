#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include "library.h"


// Function to transform main process into a daemon
static void become_daemon()
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
	for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
	{
		close(fd);
	}

	// Open log file
	openlog("mydaemon", LOG_PID, LOG_DAEMON);
}


int main()
{
	printf("daemon running as : %d\n", getpid());
	printf("%s\n", REPORT_DIR);
	// Transform into a daemon process
	become_daemon();
	// here code
	int monitor_pid = fork();
	if (monitor_pid == 0)
	{
		execl("monitor", "%s", REPORT_DIR, NULL); // adjust paths
		exit(EXIT_FAILURE);
	}
	// do again for backup
	int backup_pid = fork();
	if (backup_pid == 0)
	{
		execl("backup", "%s", REPORT_DIR, NULL); // adjust paths
		exit(EXIT_FAILURE);
	}

	// Main loop: log every 20 seconds
	while (1)
	{
		syslog(LOG_NOTICE, "mydaemon started");
		sleep(20);
		// break; - will cause pain
	}

	// Log termination and close log file
	syslog(LOG_NOTICE, "mydaemon terminated");
	closelog();

	return EXIT_SUCCESS;
}
