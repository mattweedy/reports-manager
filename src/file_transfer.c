#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "library.h"

#define MAX_BUF 1024
// TODO: create copy_to_dashboard() function that copies files from reports to dashboard at 11:30pm
// TODO: test backup() functionality
// TODO: test copy_to_dashboard() functionality

void copy_to_dashboard(char *src_dir, char *dst_dir) {
	DIR *dir;
	struct dirent *entry;
    char src_path[1024], dst_path[1024];

	if (!(dir = opendir(src_dir)))
		return;

	
}


void backup(char *src_dir, char *dst_dir)
{
	printf("backup running as : %d\n", getpid());
	DIR *dir;
	struct dirent *entry;
	char src_file[MAX_BUF];
	char dst_file[MAX_BUF];
	char buffer[MAX_BUF];
	int in, out;
	ssize_t len;

	while ((entry = readdir(dir)) != NULL)
	{
		if (strstr(entry->d_name, ".xml"))
		{
			// open source file
			snprintf(src_file, sizeof(src_file), "%s/%s", src_dir, entry->d_name);
			in = open(src_file, O_RDONLY);
			if (in == -1)
			{
				perror("open src_file");
				exit(EXIT_FAILURE);
			}

			// open destination file
			snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_dir, entry->d_name);
			out = open(dst_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
			if (out == -1)
			{
				perror("open dst_file");
				close(in);
				exit(EXIT_FAILURE);
			}

			// copy file
			while ((len = read(in, buffer, sizeof(buffer))) > 0)
			{
				if (write(out, buffer, len) != len)
				{
					perror("write");
					close(in);
					close(out);
					exit(EXIT_FAILURE);
				}
			}

			// check for read error
			if (len == -1)
			{
				perror("read");
				close(in);
				close(out);
				exit(EXIT_FAILURE);
			}

			// close files
			if (close(in) == -1)
			{
				perror("close in");
				exit(EXIT_FAILURE);
			}

			if (close(out) == -1)
			{
				perror("close out");
				exit(EXIT_FAILURE);
			}

			if (rename(src_file, dst_file) < 0)
			{
				perror("rename");
				exit(EXIT_FAILURE);
			}
		}
	}

	closedir(dir);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <src_dir> <dst_dir>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		backup(argv[1], argv[2]);
		sleep(10);
	}

	return 0;
}