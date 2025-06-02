#include <dirent.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys_dirent.h>

DIR *opendir(const char *name) {
    int fd = open(name, O_DIRECTORY | O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    DIR *dirp = (DIR*)malloc(sizeof(DIR));
    dirp->fd = fd;
    dirp->num_entries = 0;
    dirp->buf_size = 0;
    dirp->current = NULL;
    return dirp;
}

DIR *fdopendir(int fd) {
    DIR *dirp = (DIR*)malloc(sizeof(DIR));
    dirp->fd = fd;
    dirp->num_entries = 0;
    dirp->buf_size = 0;
    dirp->current = NULL;
    return dirp;
}

int closedir(DIR* dirp) {
    close(dirp->fd);
    free(dirp);
    return 0;
}

struct dirent *readdir(DIR* dirp) {
    if (dirp->current == NULL || dirp->current >= (struct dirent*)dirp->buf + dirp->num_entries) {
        getdents64(dirp->fd, dirp, sizeof(dirp->buf));
        dirp->current = (struct dirent*)dirp->buf;
        if (dirp->num_entries <= 0) {
            return NULL;
        }
    }

    struct dirent *entry = dirp->current;
    dirp->current = (struct dirent*)((void*)entry + entry->d_reclen);
    return entry;
}
