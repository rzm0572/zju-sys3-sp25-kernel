#ifndef _SYS_DIRENT_H_
#define _SYS_DIRENT_H_


enum {
    DT_UNKNOWN = 0,
    DT_FIFO = 1,
    DT_CHR = 2,
    DT_DIR = 4,
    DT_BLK = 6,
    DT_REG = 8,
    DT_LNK = 10,
    DT_SOCK = 12,
    DT_WHT = 14
};

struct dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[12];
};

typedef struct dir_stream {
    int fd;
    int num_entries;
    long buf_size;
    char buf[512];
    struct dirent *current;
} DIR;

#endif /* _SYS_DIRENT_H_ */
