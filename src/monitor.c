#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/inotify.h>

#include "library.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
#define TIMESTAMP_SIZE 20
#define FILE_PATH_SIZE 256

// TODO: The changes made to the department managers upload directory needs to
// 		 documented. The username of the user, the file they modified and the timestamp
// 		 should be recorded.

// TODO: No changes should be allowed to be made to the directories (upload and reporting)
//  	 while the backup/transfer is happening.

// TODO: Identify new or modified xml reports and log details of who made the changes, this
// 		 should be generated as a text file report and stored on the server.

// TODO: If a file wasn’t uploaded this should be logged in the system. (A naming convention
// 		 can be used to help with this task.

void current_timestamp(char *timestamp);
void monitor_directory(char *path);

void current_timestamp(char *timestamp)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(timestamp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
} 

void monitor_directory(char *path)
{
	printf("monitor : running as [%d]\n", getpid());

	int length, i = 0;
	int fd;
	int wd;
	char buffer[BUF_LEN];

	fd = inotify_init();

	if (fd < 0)
	{
		perror("inotify_init");
		return;
	}

	wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

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

		while (i < length)
		{
			struct inotify_event *event = (struct inotify_event *)&buffer[i];
			if (event->len)
			{
				char timestamp[TIMESTAMP_SIZE]; // Create a buffer to store the timestamp
				current_timestamp(timestamp);	// Get the current timestamp
				if (event->mask & IN_CREATE)
				{
					fprintf(logfile, "The file %s was created by %s at %s.\n", event->name, getenv("USER"), timestamp);
				}
				else if (event->mask & IN_DELETE)
				{
					fprintf(logfile, "The file %s was deleted by %s at %s.\n", event->name, getenv("USER"), timestamp);
				}
				else if (event->mask & IN_MODIFY)
				{
					fprintf(logfile, "The file %s was modified by %s at %s.\n", event->name, getenv("USER"), timestamp);
				}
			}
			i += EVENT_SIZE + event->len;
		}

		// Check if the expected file exists in the upload directory at 11:30pm
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		if (tm.tm_hour == 23 && tm.tm_min == 30)
		{
			char expected_file_path[FILE_PATH_SIZE];
			sprintf(expected_file_path, "%s/%s", path, "expected_file_name");
			if (access(expected_file_path, F_OK) == -1)
			{
				fprintf(logfile, "The expected file was not uploaded by 11:30pm.\n");
			}
		}
	}

	(void)inotify_rm_watch(fd, wd);
	(void)close(fd);
}

int main()
{
	logfile = fopen("/logs/monitor-logs.txt", "a");
	if (logfile == NULL)
	{
		perror("Error opening monitor-logs file");
		exit(EXIT_FAILURE);
	}

	monitor_directory(UPLOAD_DIR);
	fclose(logfile);
	return 0;
}