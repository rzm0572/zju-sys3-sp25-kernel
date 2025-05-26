#include <fs.h>
#include <vfs.h>
#include <mm.h>
#include <string.h>
#include <scank.h>
#include <printk.h>
#include <fat32.h>
#include <stdio.h>

struct files_struct *file_init() {
    // todo: alloc pages for files_struct, and initialize stdin, stdout, stderr
    struct files_struct *ret = (struct files_struct *)alloc_page();
    for (int i = 0; i <= 2; i++) {
        ret->fd_array[i].opened = 1;
        ret->fd_array[i].perms = (i == 0) ? FILE_READABLE : FILE_WRITABLE;
        ret->fd_array[i].cfo = 0;
        if (i == 0) {
            ret->fd_array[i].read = stdin_read;
        }
        else if (i == 1) {
            ret->fd_array[i].write = stdout_write;
        }
        else {
            ret->fd_array[i].write = stderr_write;
        }
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

int32_t file_open(struct file* file, const char* path, int flags) {
    file->opened = 1;
    file->perms = flags;
    file->cfo = 0;
    file->fs_type = get_fs_type(path);
    memcpy(file->path, path, strlen(path) + 1);

    if (file->fs_type == FS_TYPE_FAT32) {
        file->lseek = fat32_lseek;
        file->write = fat32_write;
        file->read = fat32_read;
        file->fat32_file = fat32_open_file(path);
        if (file->fat32_file.cluster == 0) {
            printk("File not found: %s\n", path);
            return -1;
        }
        else return 0;
        // todo: check if fat32_file is valid (i.e. successfully opened) and return
    } else if (file->fs_type == FS_TYPE_EXT2) {
        printk("Unsupport ext2\n");
        return -1;
    } else {
        printk("Unknown fs type: %s\n", path);
        return -1;
    }
}