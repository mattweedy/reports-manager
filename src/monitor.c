#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <signal.h>

#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/inotify.h>

#include "library.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
#define TIMESTAMP_SIZE 20
#define FILE_PATH_SIZE 256

FILE *logfile;

// TODO: If a file wasn’t uploaded this should be logged in the system. (A naming convention
// 		 can be used to help with this task.

void current_timestamp(char *timestamp);
void monitor_directory(const char *path);
const char *get_message_name(struct inotify_event *event);
void clean_exit(int sig);


void current_timestamp(char *timestamp)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(timestamp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

const char *get_message_name(struct inotify_event *event)
{
	if (event->mask & IN_ACCESS)
		return "ACCESSED";
	if (event->mask & IN_ATTRIB)
		return "METADATA_CHANGED";
	if (event->mask & IN_OPEN)
		return "OPENED";
	if (event->mask & IN_CLOSE_WRITE)
		return "WRITEABLE_CLOSED";
	if (event->mask & IN_CLOSE_NOWRITE)
		return "UNWRITEABLE_CLOSED";
	if (event->mask & IN_CREATE)
		return "CREATED";
	if (event->mask & IN_DELETE)
		return "DELETED";
	if (event->mask & IN_DELETE_SELF)
		return "SELF_DELETED";
	if (event->mask & IN_MODIFY)
		return "MODIFIED";
	if (event->mask & IN_MOVE_SELF)
		return "SELF_MOVED";
	if (event->mask & IN_MOVED_FROM)
		return "MOVED_FROM";
	if (event->mask & IN_MOVED_TO)
		return "MOVED_TO";
	return "UNDEFINED";
}


void clean_exit(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
	{
		syslog(LOG_NOTICE, "monitor : exiting monitor\n");
		fclose(logfile);
		closelog();
		exit(EXIT_SUCCESS);
	}
	else
	{
		syslog(LOG_WARNING, "monitor : DW about this bud received unknown signal\n");
	}
}


void monitor_directory(const char *path)
{
	printf("monitor : running as [%d]\n", getpid());
	signal(SIGINT, clean_exit);
	signal(SIGTERM, clean_exit);

	ssize_t length = 0;
	long unsigned int i = 0;
	int fd;
	int wd;
	char buffer[BUF_LEN];

	fd = inotify_init();

	if (fd < 0)
	{
		perror("inotify_init");
		return;
	}

	wd = inotify_add_watch(fd, path, IN_ACCESS | IN_ATTRIB | IN_MODIFY | IN_CREATE | IN_DELETE | IN_OPEN | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO);

	if (wd < 0)
	{

		perror("inotify_add_watch");
		close(fd);
		return;
	}

	while (1)
	{
		i = 0;
		length = read(fd, buffer, BUF_LEN);
		if (length < 0)
		{
			perror("read");
			break;
		}

		while ((ssize_t)i < length)
		{
			struct inotify_event *event = (struct inotify_event *)&buffer[i];
			if (event->len > 0)
			{
				char timestamp[TIMESTAMP_SIZE]; // Create a buffer to store the timestamp
				current_timestamp(timestamp);	// Get the current timestamp

				fprintf(logfile, "The file %s was %s by %s at %s.\n", event->name, get_message_name(event), getenv("USER"), timestamp);

				fflush(logfile);
			}
			i += EVENT_SIZE + event->len;
		}

		// Check if the expected file exists in the upload directory at 11:30pm
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		if (tm.tm_hour == 23 && tm.tm_min == 30)
		{
			char expected_file_path[FILE_PATH_SIZE];
			sprintf(expected_file_path, "%s/%s", path, "");
			if (access(expected_file_path, F_OK) == -1)
			{
				fprintf(logfile, "The expected file was not uploaded by 11:30pm.\n");
			}
		}
	}

	(void)inotify_rm_watch(fd, wd);
	(void)close(fd);
}


int main(void)
{
	openlog("monitor", LOG_PID | LOG_CONS, LOG_DAEMON);
	logfile = fopen("/var/log/reports_manager/monitor-logs.txt", "r");
	if (logfile == NULL)
	{
		syslog(LOG_WARNING, "monitor : reading monitor-logs file failed: %s", strerror(errno));
		logfile = fopen("/var/log/reports_manager/monitor-logs.txt", "w");
		if (logfile == NULL)
		{
			syslog(LOG_CRIT, "monitor : creating monitor-logs file failed: %s", strerror(errno));

			exit(EXIT_FAILURE);
		}
	}
	else
	{
		syslog(LOG_INFO, "monitor : using existing monitor-logs file");
	}

	fclose(logfile);

	logfile = fopen("/var/log/reports_manager/monitor-logs.txt", "a");

	if (logfile == NULL)
	{
		perror("Error opening monitor-logs file");
		exit(EXIT_FAILURE);
	}

	monitor_directory(UPLOAD_DIR);
	fclose(logfile);
	return EXIT_SUCCESS;
}