#include <fs.h>
#include <vfs.h>
#include <mm.h>
#include <string.h>
#include <scank.h>
#include <printk.h>
#include <fat32.h>
#include <stdio.h>
#include <fcntl.h>

struct files_struct *file_init() {
    // todo: alloc pages for files_struct, and initialize stdin, stdout, stderr
    struct files_struct *ret = (struct files_struct *)alloc_page();
    ret->fd_array[0].opened = 1;
    ret->fd_array[0].perms = FILE_READABLE;
    ret->fd_array[0].cfo = 0;
    ret->fd_array[0].lseek = NULL;
    ret->fd_array[0].write = NULL;
    ret->fd_array[0].read = stdin_read;

    ret->fd_array[1].opened = 1;
    ret->fd_array[1].perms = FILE_WRITABLE;
    ret->fd_array[1].cfo = 0;
    ret->fd_array[1].lseek = NULL;
    ret->fd_array[1].write = stdout_write;
    ret->fd_array[1].read = NULL;

    ret->fd_array[2].opened = 1;
    ret->fd_array[2].perms = FILE_WRITABLE;
    ret->fd_array[2].cfo = 0;
    ret->fd_array[2].lseek = NULL;
    ret->fd_array[2].write = stderr_write;
    ret->fd_array[2].read = NULL;

    for (int i = 3; i < MAX_FILE_NUMBER; i++) {
        ret->fd_array[i].opened = 0;
        ret->fd_array[i].perms = 0;
        ret->fd_array[i].cfo = 0;
        ret->fd_array[i].lseek = NULL;
        ret->fd_array[i].write = NULL;
        ret->fd_array[i].read = NULL;
    }

    return ret;
}

uint32_t get_fs_type(const char *filename) {
    uint32_t ret;
    if (memcmp(filename, "/fat32/", 7) == 0) {
        ret = FS_TYPE_FAT32;
    } else if (memcmp(filename, "/ext2/", 6) == 0) {
        ret = FS_TYPE_EXT2;
    } else {
        ret = -1;
    }
    return ret;
}

uint32_t get_perm(int flags) {
    uint32_t ret = 0;
    if (flags & O_RDONLY) {
        ret |= FILE_READABLE;
    }
    if (flags & O_WRONLY) {
        ret |= FILE_WRITABLE;
    }
    return ret;
}

int32_t file_open(struct file* file, const char* path, int flags) {
    file->opened = 1;
    file->perms = get_perm(flags);
    file->cfo = 0;
    file->fs_type = get_fs_type(path);
    file->flags = flags;
    memcpy(file->path, path, strlen(path) + 1);

    if (file->fs_type == FS_TYPE_FAT32) {
        file->lseek = fat32_lseek;
        file->write = fat32_write;
        if (flags & O_DIRECTORY) {
            file->read = fat32_read_dir;
            file->fat32_file = fat32_open_dir(path);
            printk(MSG("file", "open dir, cluster: %d\n"), file->fat32_file.cluster);
        } else {
            file->read = fat32_read;
            file->fat32_file = fat32_open_file(path);
            printk(MSG("file", "open file, cluster: %d\n"), file->fat32_file.cluster);
        }
        if (file->fat32_file.cluster == 0) {
            printk(MSG("file", ": %s\n"), path);
            return -1;
        }
        else return 0;
        // todo: check if fat32_file is valid (i.e. successfully opened) and return
    } else if (file->fs_type == FS_TYPE_EXT2) {
        printk(ERR("file", "Unsupport ext2\n"));
        return -1;
    } else {
        printk(ERR("file", "Unknown fs type: %s\n"), path);
        return -1;
    }
}