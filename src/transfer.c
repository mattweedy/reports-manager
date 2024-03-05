#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

void transfer_files(const char *src_dir, const char *dst_dir) {
    DIR *dir;
    struct dirent *entry;
    char src_path[1024], dst_path[1024];

    if (!(dir = opendir(src_dir)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, entry->d_name);

            rename(src_path, dst_path);
        }
    }

    closedir(dir);
}

int main() {
    const char *src_dir = "/path/to/upload/directory";
    const char *dst_dir = "/path/to/reporting/directory";

    transfer_files(src_dir, dst_dir);

    return 0;
}