#ifndef _FCNTL_H_
#define _FCNTL_H_

#define AT_FDCWD    -100 
#define O_RDONLY    0x0001
#define O_WRONLY    0x0002
#define O_RDWR      0x0003

#define O_CREAT     0x0100
#define O_EXCL      0x0200
#define O_NOCTTY    0x0400

#define O_TRUNC     0x1000
#define O_APPEND    0x2000
#define O_NONBLOCK  0x4000

#define O_LARGEFILE 0x100000
#define O_DIRECTORY 0x200000

#define SEEK_SET    0x0000
#define SEEK_CUR    0x0001
#define SEEK_END    0x0002

#endif /* _FCNTL_H_ */
