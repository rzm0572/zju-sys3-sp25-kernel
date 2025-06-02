#include <proc.h>
#include <stdio.h>
#include <string.h>
#include <ksyscalls.h>
#include <fat32.h>
#include <fs.h>
#include <printk.h>
#include <mman.h>
#include <sys_dirent.h>

extern struct task_struct *current;
extern uint64_t num_tasks;

struct ksyscall_table syscall_table[] = {
    [__NR_open] = {2, (syscall_handler_t)sys_open},
    [__NR_close] = {1, (syscall_handler_t)sys_close},
    [__NR_lseek] = {3, (syscall_handler_t)sys_lseek},
    [__NR_read] = {3, (syscall_handler_t)sys_read},
    [__NR_write] = {3, (syscall_handler_t)sys_write},
    [__NR_getpid] = {0, (syscall_handler_t)sys_getpid},
    [__NR_clone] = {1, (syscall_handler_t)sys_clone}
};

long sys_openat(int dfd, const char *path, int flags) {
    (void)dfd;
    int res = -1;
    for (int i = 0; i < MAX_FILE_NUMBER; i++) {
        if (current->files->fd_array[i].opened) {
            continue;
        }
        // printk("%lx\n", &current->files->fd_array[i]);
        res = file_open(&current->files->fd_array[i], path, flags);
        // printk("%s %d %d\n", path, i, res);
        if (!res) {
            current->files->fd_array[i].opened = 1;
            return (long) i;
        }
        else if (res == -1) {
            return -1;
        }
        break;
    }
    return -1;
}

long sys_open(const char *filename, int flags) {
    return sys_openat(AT_FDCWD, filename, flags);
}

long sys_close(int fd) {
    if (fd < 0 || fd >= MAX_FILE_NUMBER) {
        return -1;
    }
    struct file *file = &current->files->fd_array[fd];
    if (!file->opened) {
        return -1;
    }
    file->opened = 0;
    file->cfo = 0;
    return 0;
}

long sys_lseek(int fd, long offset, int whence) {
    if (fd < 0 || fd >= MAX_FILE_NUMBER) {
        return -1;
    }
    struct file *file = &current->files->fd_array[fd];
    if (!file->opened) {
        return -1;
    }
    if (file->lseek == NULL) {
        return -1;
    }
    return file->lseek(file, offset, whence);
}

long sys_read(unsigned fd, char *buff, size_t count) {
    if (fd < 0 || fd >= MAX_FILE_NUMBER) {
        return -1;
    }
    if (fd <= 2) {
        FILE* in = &__iob[fd];
        if (in->read == NULL) {
            return -1;
        }
        return in->read(in, buff, count);
    }
    
    struct file *file = &current->files->fd_array[fd];
    if (!file->opened) {
        printk("File not opened: %d\n", fd);
        return ERROR_FILE_NOT_OPEN;
    }
    else if (!(file->perms & FILE_READABLE)) {
        printk("File not readable: %d\n", fd);
        return -1;
    }
    else {
        return file->read(file, buff, count);
    }
}

long sys_write(unsigned fd, const char *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FILE_NUMBER) {
        return -1;
    }

    if (fd <= 2) {
        FILE* out = &__iob[fd];
        if (out->write == NULL) {
            return -1;
        }

        return out->write(out, buf, count);
    }
    struct file *file = &current->files->fd_array[fd];
    if (!file->opened) {
        printk("File not opened: %d\n", fd);
        return ERROR_FILE_NOT_OPEN;
    }
    else if (!(file->perms & FILE_WRITABLE)) {
        printk("File not writable: %d\n", fd);
        return -1;
    }
    else {
        return file->write(file, buf, count);
    }
}

long sys_getpid(void) {
    return current->pid;
}

long sys_clone(struct pt_regs *regs) {
    return do_fork(regs);
}

long sys_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    if (flags & MAP_ANONYMOUS) {
        return (long)do_mmap(current->mm, addr, len, to_vm_flags(prot, flags), NULL, 0, 0);
    } else {  // TODO: not support yets
        struct file *file = &current->files->fd_array[fd];
        return (long)do_mmap(current->mm, addr, len, to_vm_flags(prot, flags), file, offset, len);
    }
}

long sys_getdents64(int fd, void *dirp, size_t count) {
    if (fd < 0 || fd >= MAX_FILE_NUMBER) {
        return -1;
    }

    struct file *file = &current->files->fd_array[fd];
    if (!file->opened) {
        return -1;
    } else if (!(file->perms & FILE_READABLE)) {
        return -1;
    }

    DIR *dir = (DIR*)dirp;
    dir->buf_size = file->read(file, dir->buf, count);
    if (dir->buf_size <= 0) {
        dir->num_entries = 0;
        return -1;
    }
    dir->num_entries = dir->buf_size / sizeof(struct dirent);
    return dir->buf_size;
}
