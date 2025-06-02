#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <mman.h>
#include <malloc.h>
#include <fcntl.h>
#include <dirent.h>

// define some tests
#define PFH1 1001
#define PFH2 1002
#define FORK1 1101
#define FORK2 1102
#define FORK3 1103
#define FORK4 1104
#define SHELL 1201
#define MALLOC1 1211
#define MALLOC2 1212
#define CAT_BUF_SIZE 509

#if defined(USER_MAIN) && !(USER_MAIN > 1000 && USER_MAIN < 1300)
#warning "Invalid definition of USER_MAIN"
#undef USER_MAIN
#endif

#ifndef USER_MAIN
// 你可以修改这一行来提供代码高亮
#define USER_MAIN PFH1
#endif

#define DELAY_TIME 1247

static uint64_t user_clock(void) {
  uint64_t ret;
  asm volatile("rdtime %0" : "=r"(ret));
  return ret / 10;
}

static void delay(unsigned long ms) {
  uint64_t prev_clock = user_clock();
  while (user_clock() - prev_clock < ms * 1000)
    ;
}

#if USER_MAIN == PFH1

int main(void) {
  register const void *const sp asm("sp");

  while (1) {
    printf("\x1b[44m[U]\x1b[0m [PID = %d, sp = %p]\n", getpid(), sp);
    delay(DELAY_TIME);
  }
}

#elif USER_MAIN == PFH2

const char *const xdigits = "0123456789abcdef";
char space[0x2000] __attribute__((align(0x1000)));
size_t i;

int main(void) {
  while (1) {
    i = 0;
    printf("\x1b[44m[U]\x1b[0m [PID = %d] ", getpid());
    while (i < sizeof(space)) {
      space[i] = xdigits[i % 16];
      printf("\x1b[4%cm%c\x1b[0m", xdigits[rand() % 8], space[i]);
      i++;
      delay(1);
    }
    printf("\n");
  }
}

#elif USER_MAIN == FORK1

int var = 0;

int main(void) {
  pid_t pid = fork();
  const char *ident = pid ? "PARN" : "CHLD";

  while (1) {
    printf("\x1b[44m[U-%s]\x1b[0m [PID = %d] var = %d\n", ident, getpid(), var++);
    delay(DELAY_TIME / 2 + rand() % DELAY_TIME);
  }
}

#elif USER_MAIN == FORK2

int var = 0;
char space[0x2000] __attribute__((align(0x1000)));

int main(void) {
  for (int i = 0; i < 3; i++) {
    printf("\x1b[44m[U]\x1b[0m [PID = %d] var = %d\n", getpid(), var++);
    delay(DELAY_TIME);
  }

  memcpy(&space[0x1000], "ZJU Sys3 Lab5", 14);

  pid_t pid = fork();
  const char *ident = pid ? "PARN" : "CHLD";

  printf("\x1b[44m[U-%s]\x1b[0m [PID = %d] Message: %s\n", ident, getpid(), &space[0x1000]);
  while (1) {
    printf("\x1b[44m[U-%s]\x1b[0m [PID = %d] var = %d\n", ident, getpid(), var++);
    delay(DELAY_TIME / 2 + rand() % DELAY_TIME);
  }
}

#elif USER_MAIN == FORK3

int var = 0;

int main(void) {
  printf("\x1b[44m[U]\x1b[0m [PID = %d] var = %d\n", getpid(), var++);
  fork();
  fork(); // multiple references to one page

  printf("\x1b[44m[U]\x1b[0m [PID = %d] var = %d\n", getpid(), var++);
  fork();

  while (1) {
    printf("\x1b[44m[U]\x1b[0m [PID = %d] var = %d\n", getpid(), var++);
    delay(DELAY_TIME / 2 + rand() % DELAY_TIME);
  }
}

#elif USER_MAIN == FORK4

#define LARGE 1000

int var = 0;
long bigarr[LARGE] __attribute__((align(0x1000))) = {};

int fib(int times) {
  if (times <= 2) {
    return 1;
  } else {
    return fib(times - 1) + fib(times - 2);
  }
}

const char *suffix(int num) {
  num %= 100;
  int i = num % 10;
  if (i == 1 && num != 11) {
    return "st";
  } else if (i == 2 && num != 12) {
    return "nd";
  } else if (i == 3 && num != 13) {
    return "rd";
  } else {
    return "th";
  }
}

int main(void) {
  for (int i = 0; i < LARGE; i++) {
    bigarr[i] = 3 * i + 1;
  }

  pid_t pid = fork();
  const char *ident = pid ? "PARN" : "CHLD";
  printf("\x1b[44m[U]\x1b[0m fork returns %d\n", pid);

  while (1) {
    var = 0;
    while (var < LARGE) {
      printf("\x1b[44m[U-%s]\x1b[0m [PID = %d] the %d%s fibonacci number is %d and "
             "the %d%s number in the big array is %ld\n",
             ident, getpid(), var, suffix(var), fib(var), LARGE - 1 - var, suffix(LARGE - 1 - var),
             bigarr[LARGE - 1 - var]);
      var++;
      delay(100);
    }
  }
}

#elif USER_MAIN == SHELL
char string_buf[2048];
char filename[2048];
int atoi(const char *str) {
  int res = 0, len = strlen(str);
  for (int i = 0; i < len; i++) {
    res = res * 10 + str[i] - '0';
  }
  return res;
}

char *get_param(char *cmd) {
  while (*cmd == ' ') {
    cmd ++;
  }
  int pos = 0;
  while (*cmd != '\0' && *cmd != ' ') {
    string_buf[pos++] = *(cmd++);
  }
  string_buf[pos] = '\0';
  return string_buf;
}

char *get_string(char *cmd) {
  while (*cmd == ' ') {
    cmd ++;
  }
  if (*cmd == '"') {
    cmd ++;
    int pos = 0;
    while (*cmd != '"') {
      string_buf[pos++] = *(cmd++);
    }
    string_buf[pos] = '\0';
    return string_buf;
  }
  else {
    return get_param(cmd);
  }
}

int parse_cmd(char *cmd, int len) {
  if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o') {
    cmd += 4;
    char *echo_content = get_string(cmd);
    len = strlen(echo_content);
    cmd += len;
    write(1, echo_content, len);
    write(1, "\n", 1);
  }
  else if (cmd[0] == 'l' && cmd[1] == 's') {
    cmd += 2;
    while (*cmd == ' ' && *cmd != '\0') {
      cmd ++;
    }
    char *filename = get_param(cmd);
    int fd = open(filename, O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
      printf("can't open file: %s\n", filename);
      return 0;
    }
    DIR* dirp = fdopendir(fd);
    struct dirent *ent;
    while ((ent = readdir(dirp)) != NULL) {
      printf("%s\n", ent->d_name);
    }
    closedir(dirp);
  }
  else if (cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't') {
    char *filename = get_param(cmd + 3);
    char last_char = '\0';
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
      printf("can't open file: %s\n", filename);
      return 0;
    }
    char cat_buf[CAT_BUF_SIZE];
    // printf("%d\n", fd);
    while (1) {
      int num_chars = read(fd, cat_buf, CAT_BUF_SIZE);
      printf("num_chars = %d\n", num_chars);
      if (!num_chars) {
        if (last_char != '\n') {
          printf("$\n");
        }
        break;
      }
      for (int i = 0; i < num_chars; i++) {
        if (cat_buf[i] == '\0') {
          write(1, "x", 1);
        }
        else {
          write(1, &cat_buf[i], 1);
        }
        last_char = cat_buf[i];
      }
    }
    close(fd);
  }
  else if (cmd[0] == 'e' && cmd[1] == 'd' && cmd[2] == 'i' && cmd[3] == 't') {
    cmd += 4;
    while (*cmd == ' ' && *cmd != '\0') {
      cmd++;
    }
    char *tmp = get_param(cmd);
    int len = strlen(tmp);
    char filename[len + 1];
    for (int i = 0; i < len; i++) {
      filename[i] = tmp[i];
    }
    filename[len] = '\0';
    cmd += len;
    while (*cmd == ' ' && *cmd != '\0') {
      cmd++;
    }
    tmp = get_param(cmd);
    len = strlen(tmp);
    char offset[len + 1];
    for (int i = 0; i < len; i++) {
      offset[i] = tmp[i];
    }
    offset[len] = '\0';
    cmd += len;
    while (*cmd == ' ' && *cmd != '\0') {
      cmd++;
    }
    tmp = get_string(cmd);
    len = strlen(tmp);
    char content[len + 1];
    for (int i = 0; i < len; i++) {
      content[i] = tmp[i];
    }
    content[len] = '\0';
    cmd += len;
    int offset_int = atoi(offset);
    int fd = open(filename, O_RDWR);
    lseek(fd, offset_int, SEEK_SET);
    write(fd, content, len);
    close(fd);
  }
  else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') {
    printf("Exiting shell...\n");
    return 1;
  }
  else {
    printf("Command not supported: %s\n", cmd);
    return 0;
  }
  return 0;
}

int main() {
  // printf("Now I'm here!\n");
  // write(1, "Hello, stdout!\n", 15);
  // write(2, "Hello, stderr!\n", 15);
  char read_buf[2], line_buf[128];
  int char_in_line = 0;
  printf(YELLOW "SHELL > " CLEAR); 
  while (1) {
    read(0, read_buf, 1);
    if (read_buf[0] == '\r') {
      write(1, "\n", 1);
    }
    else if (read_buf[0] == 0x7f) {
      if (char_in_line > 0) {
        write(1, "\b \b", 3);
        char_in_line--;
      }
      continue;
    }
    write(1, read_buf, 1);
    if (read_buf[0] == '\r') {
      line_buf[char_in_line] = '\0';
      int res = parse_cmd(line_buf, char_in_line);
      if (res == 1) {
        while (1) ;
      }
      char_in_line = 0;
      printf(YELLOW "SHELL > " CLEAR);
    }
    else {
      line_buf[char_in_line++] = read_buf[0];
    }
  }
  return 0;
}

#elif USER_MAIN == MALLOC1

extern uint8_t _edata_user[];

#define PGSIZE 0x1000
#define PGROUNDDOWN(addr) ((addr) & ~(PGSIZE - 1))
#define PGROUNDUP(addr) PGROUNDDOWN((addr) + PGSIZE - 1)

int main() {
  void *mmap_start = (void*)PGROUNDUP((uint64_t) _edata_user) + PGSIZE;
  size_t mmap_size = 4 * PGSIZE;
  uint64_t *mmap_ptr = (uint64_t*)mmap(mmap_start, mmap_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  for (uint64_t i = 0; i < mmap_size / sizeof(uint64_t); i++) {
    mmap_ptr[i] = i;
  }
  printf("mmap_ptr = %p\n", mmap_ptr);
  printf("mmap_ptr[0] = %lu\n", mmap_ptr[0]);
  printf("mmap_ptr[-1] = %lu\n", mmap_ptr[mmap_size / sizeof(uint64_t) - 1]);

  munmap(mmap_start, mmap_size);

  while (1)
    ;
  
  return 0;
}

#elif USER_MAIN == MALLOC2

int main() {
  int N = 8178;
  printf("malloc %d bytes\n", N * sizeof(int));
  int *a = (int*)malloc(N * sizeof(int));
  for (int i = 0; i < N; i++) {
    a[i] = i;
  }
  printf("a = %p\n", a);
  printf("a[0] = %d\n", a[0]);
  printf("&a[N-1] = %p\n", &a[N - 1]);
  printf("a[N-1] = %d\n", a[N - 1]);

  free(a);

  int *b = (int*)malloc(N * sizeof(int));
  printf("b = %p\n", b);
  printf("b[0] = %d\n", b[0]);
  printf("&b[N-1] = %p\n", &b[N - 1]);
  printf("b[N-1] = %d\n", b[N - 1]);

  free(b);

  char *c[] = {
    (char*)malloc(1),
    (char*)malloc(1),
    (char*)malloc(1),
    (char*)malloc(2),
    (char*)malloc(512)
  };

  printf("c[0] = %p\n", c[0]);
  printf("c[1] = %p\n", c[1]);
  printf("c[2] = %p\n", c[2]);
  printf("c[3] = %p\n", c[3]);
  printf("c[4] = %p\n", c[4]);

  free(c[0]);
  free(c[1]);
  free(c[2]);
  free(c[3]);
  free(c[4]);

  while (1)
    ;

  return 0;
}

#endif
