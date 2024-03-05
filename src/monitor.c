#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>

#include "library.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

// TODO: The changes made to the department managers upload directory needs to
// 		 documented. The username of the user, the file they modified and the timestamp
// 		 should be recorded.

// TODO: No changes should be allowed to be made to the directories (upload and reporting)
//  	 while the backup/transfer is happening.

// TODO: Identify new or modified xml reports and log details of who made the changes, this
// 		 should be generated as a text file report and stored on the server.

// TODO: If a file wasn’t uploaded this should be logged in the system. (A naming convention
// 		 can be used to help with this task.




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
	}

	wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);

	length = read(fd, buffer, BUF_LEN);

	if (length < 0)
	{
		perror("read");
	}

	while (i < length)
	{
		struct inotify_event *event = (struct inotify_event *)&buffer[i];
		if (event->len)
		{
			if (event->mask & IN_CREATE)
			{
				printf("The file %s was created.\n", event->name);
			}
			else if (event->mask & IN_DELETE)
			{
				printf("The file %s was deleted.\n", event->name);
			}
			else if (event->mask & IN_MODIFY)
			{
				printf("The file %s was modified.\n", event->name);
			}
		}
		i += EVENT_SIZE + event->len;
	}

	(void)inotify_rm_watch(fd, wd);
	(void)close(fd);
}

int main()
{
	return 0;
}