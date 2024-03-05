#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>

#include "library.h"

#define MAX_BUF 1024

void copy_to_dashboard(char *src_dir, char *dst_dir);
void backup(char *src_dir, char *dst_dir);

void copy_to_dashboard(char *src_dir, char *dst_dir)
{
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char src_file[MAX_BUF];
	char dst_file[MAX_BUF];
	char buffer[MAX_BUF];
	int in, out;
	ssize_t len;

	dir = opendir(src_dir);
	if (dir == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dir)) != NULL) {
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		// construct source path
		snprintf(src_file, sizeof(src_file), "%s/%s", src_dir, entry->d_name);

		// check if entry is a directory
		if (stat(src_file, &entry_stat) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}

		// construct destination path
		snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_dir, entry->d_name);

		if (S_ISDIR(entry_stat.st_mode)) {
			// create the directory in dst_dir, if it doesn't exist
			mkdir(dst_file, 0755);
			// recurse into the directory
			copy_to_dashboard(src_file, dst_file);
		} else {
			// open source file
			in = open(src_file, O_RDONLY);
			if (in == -1) {
				perror("open");
				exit(EXIT_FAILURE);
			}

			// open destination file
			out = open(dst_file, O_WRONLY | O_CREAT, 0644);
			if (out == -1) {
				perror("open");
				close(in);
				exit(EXIT_FAILURE);
			}

			// copy file
			while ((len = read(in, buffer, sizeof(buffer))) > 0) {
				if (write(out, buffer, len) != len) {
					perror("write");
					close(in);
					close(out);
					exit(EXIT_FAILURE);
				}
			}

			if (len == -1) {
				perror("read");
				close(in);
				close(out);
				exit(EXIT_FAILURE);
			}

			// close files
			if (close(in) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
			if (close(out) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
		}
	}

	closedir(dir);
}

void backup(char *src_dir, char *dst_dir)
{
	// TODO: REMEMBER BACKUP MOVES FROM DASHBOARD -> BACKUP
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char src_file[MAX_BUF];
	char dst_file[MAX_BUF];
	char buffer[MAX_BUF];
	int in, out;
	ssize_t len;

	dir = opendir(src_dir);
	if (dir == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dir)) != NULL) {
		// printf("file name: %s\n", entry->d_name);
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		// open source file
		snprintf(src_file, sizeof(src_file), "%s/%s", src_dir, entry->d_name);

		// check if entry is a directory
		if (stat(src_file, &entry_stat) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}

		// open destination file
		snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_dir, entry->d_name);

		if (S_ISDIR(entry_stat.st_mode)) {
			// create dir if doesnt exist
			mkdir(dst_file, 0755);
			// recurse into dir
			backup(src_file, dst_file);
		} else {
			// move file
			if (rename(src_file, dst_file) == -1) {
				perror("rename");
				exit(EXIT_FAILURE);
			}
		}
	}

	closedir(dir);
}

int main(int argc, char *argv[])
{
	printf("file_transfer : running as [%d]\n", getpid());
	int opt;
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s [-t]/[-b] <src_dir> <dst_dir>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "tb")) != -1)
	{
		switch (opt)
		{
		case 'b':
			printf("file_transfer : Backup option selected\n");
			backup(argv[2], argv[3]);
			exit(EXIT_SUCCESS);
		case 't':
			printf("file_transfer : Copy to dashboard option selected\n");
			copy_to_dashboard(argv[2], argv[3]);
			exit(EXIT_SUCCESS);
		case 'd':
			syslog(LOG_INFO, "file_transfer : Daemon option selected"); // make syslog
			break;
		default:
			fprintf(stderr, "Usage: %s [-t]/[-b] <src_dir> <dst_dir>\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	for (;;) { // this is for sitting and waiting when daemon calls it
		// code for checking if time is 1am
		// call copy_to_dashboard()
		
		// code for checking if time is 3am
		
		sleep(20);
	}

	return 0;
}