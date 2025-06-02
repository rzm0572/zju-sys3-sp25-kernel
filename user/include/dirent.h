#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <unistd.h>
#include <sys_dirent.h>

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
int closedir(DIR* dirp);
struct dirent *readdir(DIR* dirp);

#endif /* _DIRENT_H_ */
