#ifndef _MMAN_H_
#define _MMAN_H_

#define PROT_READ   0x01
#define PROT_WRITE  0x02
#define PROT_EXEC   0x04
#define PROC_NONE   0x00

#define MAP_SHARED     0x01
#define MAP_PRIVATE    0x02
#define MAP_FILE       0x00
#define MAP_FIXED      0x10
#define MAP_ANONYMOUS  0x20

#define MAP_ANON       MAP_ANONYMOUS

#define MAP_FAILED     (void*)-1

#endif /* _MMAN_H_ */