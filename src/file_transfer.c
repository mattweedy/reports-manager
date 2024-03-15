#include <time.h>
#include <errno.h>
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
#include <dirent.h>
#include <fnmatch.h>

#include "library.h"

#define MAX_BUF 1024

void lock_directory(const char *path);
void unlock_directory(const char *path);
long check_files_uploaded(const char *path, const char *department);
void call_backup(const char *src_dir, const char *dst_dir);
void backup(const char *src_dir, const char *dst_dir);
void transfer_to_dashboard(const char *src_dir, const char *dst_dir);

void lock_directory(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char sub_path[MAX_BUF];

	dir = opendir(path);
	if (dir == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dir)) != NULL)
	{
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		// construct source path
		snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);

		// change perms to read-only
		if (chmod(sub_path, S_IRUSR | S_IRGRP | S_IROTH) == -1)
		{
			perror("chmod");
			exit(EXIT_FAILURE);
		}

		// check if entry is a directory
		if (stat(sub_path, &entry_stat) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}

		if (S_ISDIR(entry_stat.st_mode))
		{
			// recurse into the directory
			lock_directory(sub_path);
		}
	}
}

void unlock_directory(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char sub_path[MAX_BUF];

	dir = opendir(path);
	if (dir == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dir)) != NULL)
	{
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}

		// construct source path
		snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);

		// change perms to read-write
		if (chmod(sub_path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH) == -1)
		{
			perror("chmod");
			exit(EXIT_FAILURE);
		}

		// check if entry is a directory
		if (stat(sub_path, &entry_stat) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}

		if (S_ISDIR(entry_stat.st_mode))
		{
			// recurse into the directory
			unlock_directory(sub_path);
		}
	}
}

long check_files_uploaded(const char *path, const char *department)
{
	int match_count = 0;
	DIR *dir;
	struct dirent *entry;
	char path_temp[1024];
	snprintf(path_temp, sizeof(path_temp), "%s/%s", path, department);
	dir = opendir(path_temp);
	if (dir == NULL)
	{
		syslog(LOG_ERR, "file_transfer: (%s) wouldn't open", path_temp);
		return -1;
	}

	while ((entry = readdir(dir)) != NULL)
	{
		// if (entry->d_name == "." || entry->d_name == "..") continue;
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}

		syslog(LOG_DEBUG, "Checking %s against %s", entry->d_name, "*.xml"); // change
		int result = fnmatch("*.xml", entry->d_name, FNM_PATHNAME);
		if (result == 0)
		{
			match_count++;
		}
		else if (result != FNM_NOMATCH)
		{
			syslog(LOG_ERR, "Error in fnmatch: %s", strerror(errno));
			return -1;
		}
	}
	return match_count;
}

void call_backup(const char *src_dir, const char *dst_dir)
{
	printf("file_transfer : starting backup...\n");
	backup(src_dir, dst_dir);
	printf("file_transfer : backup complete\n");
}

void backup(const char *src_dir, const char *dst_dir) // copies files from reporting -> backup
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

	lock_directory(src_dir);
	unlock_directory(dst_dir);

	while ((entry = readdir(dir)) != NULL)
	{
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		// construct source path
		snprintf(src_file, sizeof(src_file), "%s/%s", src_dir, entry->d_name);

		// check if entry is a directory
		if (stat(src_file, &entry_stat) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}

		// construct destination path
		snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_dir, entry->d_name);

		if (S_ISDIR(entry_stat.st_mode))
		{
			// create the directory in dst_dir, if it doesn't exist
			mkdir(dst_file, 0220);
			// recurse into the directory
			backup(src_file, dst_file);
		}
		else
		{
			// open source file
			in = open(src_file, O_RDONLY);
			if (in == -1)
			{
				perror("open");
				exit(EXIT_FAILURE);
			}

			// open destination file
			out = open(dst_file, O_WRONLY | O_CREAT, 0644);
			if (out == -1)
			{
				perror("open");
				close(in);
				exit(EXIT_FAILURE);
			}

			// copy file
			while ((len = read(in, buffer, sizeof(buffer))) > 0)
			{
				if (write(out, buffer, (size_t)len) != len)
				{
					perror("write");
					close(in);
					close(out);
					exit(EXIT_FAILURE);
				}
			}

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
				perror("close");
				exit(EXIT_FAILURE);
			}
			if (close(out) == -1)
			{
				perror("close");
				exit(EXIT_FAILURE);
			}
		}
	}

	closedir(dir);
	unlock_directory(src_dir);
	lock_directory(dst_dir);
}

void transfer_to_dashboard(const char *src_dir, const char *dst_dir) // removes files from upload sends -> reporting (dashboard)
{
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char src_file[MAX_BUF];
	char dst_file[MAX_BUF];

	dir = opendir(src_dir);
	if (dir == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	lock_directory(src_dir);
	lock_directory(dst_dir);

	while ((entry = readdir(dir)) != NULL)
	{
		// skip curr dir and parent dir
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		// open source file
		snprintf(src_file, sizeof(src_file), "%s/%s", src_dir, entry->d_name);

		// check if entry is a directory
		if (stat(src_file, &entry_stat) == -1)
		{
			perror("stat");
			exit(EXIT_FAILURE);
		}

		// open destination file
		snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_dir, entry->d_name);

		if (S_ISDIR(entry_stat.st_mode))
		{
			// create dir if doesnt exist
			// perms : owner rwx, group rx, others rx
			mkdir(dst_file, 0220);
			// recurse into dir
			transfer_to_dashboard(src_file, dst_file);
		}
		else
		{
			// move file
			if (rename(src_file, dst_file) == -1)
			{
				perror("rename");
				exit(EXIT_FAILURE);
			}
		}
	}

	closedir(dir);

	unlock_directory(src_dir);
	unlock_directory(dst_dir);
}

int main(int argc, char *argv[])
{
	openlog("file_transfer", LOG_PID | LOG_CONS, LOG_DAEMON);
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
			call_backup(REPORTING_DIR, BACKUP_DIR);
			exit(EXIT_SUCCESS);
		case 't':
			printf("file_transfer : Transfer to dashboard option selected\n");
			transfer_to_dashboard(UPLOAD_DIR, REPORTING_DIR);
			exit(EXIT_SUCCESS);
		case 'd':
			syslog(LOG_INFO, "file_transfer : Daemon option selected"); // make syslog
			break;
		default:
			// -d option doensn't require src_dir and dst_dir
			fprintf(stderr, "Usage: %s [-t]/[-b]/[-d]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	bool doing_backup = false;
	bool doing_transfer = false;

	for (;;)
	{ // this is for sitting and waiting when daemon calls it
		time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		char current_time[6];

		strftime(current_time, sizeof(current_time), "%H:%M", tm);

		syslog(LOG_INFO, "file_transfer : idling... %s\n", current_time);
		printf("file_transfer : idling... %s\n", current_time);

		// if it is 23:00, check if all files are uploaded

		// if it is 01:00, transfer to dashboard
		if (strcmp(current_time, TRANSFER_TIME) == 0)
		{
			if (check_files_uploaded(UPLOAD_DIR, "distribution") == 0)
				syslog(LOG_WARNING, "file_transfer : no files uploaded by distribution");
			if (check_files_uploaded(UPLOAD_DIR, "manufacturing") == 0)
				syslog(LOG_WARNING, "file_transfer : no files uploaded by manufacturing");
			if (check_files_uploaded(UPLOAD_DIR, "sales") == 0)
				syslog(LOG_WARNING, "file_transfer : no files uploaded by sales");
			if (check_files_uploaded(UPLOAD_DIR, "warehouse") == 0)
				syslog(LOG_WARNING, "file_transfer : no files uploaded by warehouse");

			syslog(LOG_INFO, "file_transfer : initiating automatic transfer to dashboard");
			transfer_to_dashboard(UPLOAD_DIR, REPORTING_DIR);
			sleep(60);
		}

		// if it is 03:00, backup
		if (strcmp(current_time, BACKUP_TIME) == 0)
		{
			syslog(LOG_INFO, "file_transfer : initiating automatic backup\n");
			call_backup(REPORTING_DIR, BACKUP_DIR);
			sleep(60);
		}
		sleep(5);
	}

	return 0;
}