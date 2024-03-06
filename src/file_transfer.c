#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>

#include "library.h"

#define MAX_BUF 1024

void lock_directory(char *path);
void unlock_directory(char *path);
void transfer_to_dashboard(char *src_dir, char *dst_dir);
void backup(char *src_dir, char *dst_dir);

void lock_directory(char *path) {
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char sub_path[MAX_BUF];

	dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dir)) != NULL) {
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		// construct source path
		snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);

		// change perms to read-only
		if (chmod(sub_path, S_IRUSR | S_IRGRP | S_IROTH) == -1) {
			perror("chmod");
			exit(EXIT_FAILURE);
		}

		// check if entry is a directory
		if (stat(sub_path, &entry_stat) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}

		if (S_ISDIR(entry_stat.st_mode)) {
			// recurse into the directory
			lock_directory(sub_path);
		}
	}
}


void unlock_directory(char *path) {
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char sub_path[MAX_BUF];

	dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dir)) != NULL) {
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// construct source path
		snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);

		// change perms to read-write
		if (chmod(sub_path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH) == -1) {
			perror("chmod");
			exit(EXIT_FAILURE);
		}

		// check if entry is a directory
		if (stat(sub_path, &entry_stat) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}

		if (S_ISDIR(entry_stat.st_mode)) {
			// recurse into the directory
			unlock_directory(sub_path);
		}
	}
}


void transfer_to_dashboard(char *src_dir, char *dst_dir)
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

	lock_directory(src_dir);
	lock_directory(dst_dir);

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
			mkdir(dst_file, 0220);
			// recurse into the directory
			transfer_to_dashboard(src_file, dst_file);
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
	unlock_directory(src_dir);
	unlock_directory(dst_dir);
}

void backup(char *src_dir, char *dst_dir)
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
	if (dir == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	printf("file_transfer : starting backup...\n");
	printf("file_transfer : locking directories...\n");

	lock_directory(src_dir);
	lock_directory(dst_dir);

	printf("file_transfer : locked directories...\n");

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
			// perms : owner rwx, group rx, others rx
			mkdir(dst_file, 0220);
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

	printf("file_transfer : unlocking directories...\n");

	unlock_directory(src_dir);
	unlock_directory(dst_dir);

	printf("file_transfer : unlocked directories...\n");
	printf("file_transfer : backup complete\n");
}

int main(int argc, char *argv[])
{
	int opt;
	if (argc == 2 && strcmp(argv[1], "-d") == 0)
	{
		syslog(LOG_INFO, "file_transfer : Daemon option selected"); // make syslog
	}
	else if (argc != 2)
	{
		// -d option doensn't require src_dir and dst_dir
		fprintf(stderr, "Usage: %s [-t]/[-b]/[-d]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("file_transfer : running as [%d]\n", getpid());

	while ((opt = getopt(argc, argv, "btd")) != -1)
	{
		switch (opt)
		{
		case 'b':
			printf("file_transfer : Backup option selected\n");
			backup(REPORT_DIR, BACKUP_DIR);
			exit(EXIT_SUCCESS);
		case 't':
			printf("file_transfer : Transfer to dashboard option selected\n");
			transfer_to_dashboard(REPORT_DIR, DASHBOARD_DIR);
			exit(EXIT_SUCCESS);
		case 'd':
			syslog(LOG_INFO, "file_transfer : Daemon option selected"); // make syslog
			break;
		default:
			// -d option doensn't require src_dir and dst_dir
			// fprintf(stderr, "Usage: %s [-t]/[-b]/([-d]) <src_dir> <dst_dir>\n", argv[0]);
			fprintf(stderr, "Usage: %s [-t]/[-b]/[-d]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	bool doing_backup = false;
	bool doing_transfer = false;

	for (;;) { // this is for sitting and waiting when daemon calls it
		time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		char current_time[6];

		strftime(current_time, sizeof(current_time), "%H:%M", tm);

		syslog(LOG_INFO, "DAEMON:file_transfer : idling... %s\n", current_time);
		printf("DAEMON:file_transfer : idling... %s\n", current_time);

		// TODO: managers must upload to /reports/ by 23:00 if not, log it

		// if it is 01:00, transfer to dashboard
		if (strcmp(current_time, "00:44") == 0) { // TODO: use <library.h> TRANSFER_TIME
			if (!doing_transfer) {
				doing_transfer = true;
				syslog(LOG_INFO, "DAEMON:file_transfer : initiating automatic transfer to dashboard\n");
				// printf("DAEMON:file_transfer : initiating automatic transfer to dashboard\n");
				transfer_to_dashboard(REPORT_DIR, DASHBOARD_DIR);
			} 
		} else if (strcmp(current_time, "00:45") == 0) {
			doing_transfer = false;
		}

		// if it is 03:00, backup
		if (strcmp(current_time, "00:45") == 0) {
			if (!doing_backup) {
				doing_backup = true;
				syslog(LOG_INFO, "DAEMON:file_transfer : initiating automatic backup\n");
				// printf("DAEMON:file_transfer : initiating automatic backup\n");
				backup(DASHBOARD_DIR, BACKUP_DIR);
			}
		} else if (strcmp(current_time, "00:46") == 0) {
			doing_backup = false;
		}
		sleep(5);
	}

	return 0;
}