
uapp.elf:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <_start>:
    .section .text.init
    .globl _start
_start:
    j main
       0:	0240006f          	j	24 <main>

0000000000000004 <user_clock>:
#include <time.h>

// TODO for you:
// try to implement the C library function clock() so that it can be
// used across the kernel and user space, be DRY :)
static uint64_t user_clock(void) {
       4:	ff010113          	addi	sp,sp,-16
       8:	00113423          	sd	ra,8(sp)
  return (uint64_t)clock() / 10;
       c:	2a0000ef          	jal	2ac <clock>
      10:	00a00593          	li	a1,10
      14:	7b9000ef          	jal	fcc <__hidden___udivdi3>
}
      18:	00813083          	ld	ra,8(sp)
      1c:	01010113          	addi	sp,sp,16
      20:	00008067          	ret

0000000000000024 <main>:

// IMPORTANT: DO NOT move global variables into main function
int i;
uint64_t prev_clock;

int main(void) {
      24:	ff010113          	addi	sp,sp,-16
      28:	00113423          	sd	ra,8(sp)
      2c:	00813023          	sd	s0,0(sp)
  register const void *const sp asm("sp");

  // lets just wait some time
  prev_clock = user_clock();
      30:	fd5ff0ef          	jal	4 <user_clock>
      34:	00001797          	auipc	a5,0x1
      38:	60a7b223          	sd	a0,1540(a5) # 1638 <prev_clock>
  while (user_clock() - prev_clock < 500000)
      3c:	fc9ff0ef          	jal	4 <user_clock>
      40:	00001797          	auipc	a5,0x1
      44:	5f87b783          	ld	a5,1528(a5) # 1638 <prev_clock>
      48:	40f50533          	sub	a0,a0,a5
      4c:	0007a7b7          	lui	a5,0x7a
      50:	11f78793          	addi	a5,a5,287 # 7a11f <seed+0x78ad7>
      54:	fea7f4e3          	bgeu	a5,a0,3c <main+0x18>
    ;

  while (1) {
    printf("\x1b[44m[U]\x1b[0m [PID = %d, sp = %p] i = %d @ %" PRIu64 "\n", getpid(), sp, ++i, prev_clock);
      58:	0f0000ef          	jal	148 <getpid>
      5c:	00050593          	mv	a1,a0
      60:	00001797          	auipc	a5,0x1
      64:	5e078793          	addi	a5,a5,1504 # 1640 <i>
      68:	0007a683          	lw	a3,0(a5)
      6c:	0016869b          	addiw	a3,a3,1
      70:	00d7a023          	sw	a3,0(a5)
      74:	00001417          	auipc	s0,0x1
      78:	5c440413          	addi	s0,s0,1476 # 1638 <prev_clock>
      7c:	00043703          	ld	a4,0(s0)
      80:	00010613          	mv	a2,sp
      84:	00001517          	auipc	a0,0x1
      88:	05450513          	addi	a0,a0,84 # 10d8 <__iob+0x30>
      8c:	074000ef          	jal	100 <printf>

    // another interesting question for you to think about:
    // why when the tasks are scheduled the second time,
    // all tasks just suddenly "lined up" with the timer interrupt?
    prev_clock = user_clock();
      90:	f75ff0ef          	jal	4 <user_clock>
      94:	00a43023          	sd	a0,0(s0)
    // printf("prev_clock = %ld\n", prev_clock);
    while (user_clock() - prev_clock < 1000000)
      98:	f6dff0ef          	jal	4 <user_clock>
      9c:	00001797          	auipc	a5,0x1
      a0:	59c7b783          	ld	a5,1436(a5) # 1638 <prev_clock>
      a4:	40f50533          	sub	a0,a0,a5
      a8:	000f47b7          	lui	a5,0xf4
      ac:	23f78793          	addi	a5,a5,575 # f423f <seed+0xf2bf7>
      b0:	fea7f4e3          	bgeu	a5,a0,98 <main+0x74>
      b4:	fa5ff06f          	j	58 <main+0x34>

00000000000000b8 <fileno>:
  return (int)write(fileno(fp), buf, len);
}

int fileno(FILE *restrict f) {
  return f->fd;
}
      b8:	00052503          	lw	a0,0(a0)
      bc:	00008067          	ret

00000000000000c0 <printf_syscall_write>:
static int printf_syscall_write(FILE *restrict fp, const void *restrict buf, size_t len) {
      c0:	fe010113          	addi	sp,sp,-32
      c4:	00113c23          	sd	ra,24(sp)
      c8:	00813823          	sd	s0,16(sp)
      cc:	00913423          	sd	s1,8(sp)
      d0:	00058413          	mv	s0,a1
      d4:	00060493          	mv	s1,a2
  return (int)write(fileno(fp), buf, len);
      d8:	fe1ff0ef          	jal	b8 <fileno>
      dc:	00048613          	mv	a2,s1
      e0:	00040593          	mv	a1,s0
      e4:	078000ef          	jal	15c <write>
}
      e8:	0005051b          	sext.w	a0,a0
      ec:	01813083          	ld	ra,24(sp)
      f0:	01013403          	ld	s0,16(sp)
      f4:	00813483          	ld	s1,8(sp)
      f8:	02010113          	addi	sp,sp,32
      fc:	00008067          	ret

0000000000000100 <printf>:

int printf(const char *restrict fmt, ...) {
     100:	fa010113          	addi	sp,sp,-96
     104:	00113c23          	sd	ra,24(sp)
     108:	02b13423          	sd	a1,40(sp)
     10c:	02c13823          	sd	a2,48(sp)
     110:	02d13c23          	sd	a3,56(sp)
     114:	04e13023          	sd	a4,64(sp)
     118:	04f13423          	sd	a5,72(sp)
     11c:	05013823          	sd	a6,80(sp)
     120:	05113c23          	sd	a7,88(sp)
  va_list ap;
  va_start(ap, fmt);
     124:	02810613          	addi	a2,sp,40
     128:	00c13423          	sd	a2,8(sp)
  int ret = vfprintf(stdout, fmt, ap);
     12c:	00050593          	mv	a1,a0
     130:	00001517          	auipc	a0,0x1
     134:	f8850513          	addi	a0,a0,-120 # 10b8 <__iob+0x10>
     138:	5d5000ef          	jal	f0c <vfprintf>
  va_end(ap);
  return ret;
}
     13c:	01813083          	ld	ra,24(sp)
     140:	06010113          	addi	sp,sp,96
     144:	00008067          	ret

0000000000000148 <getpid>:
#include <stdint.h>
#include <time.h>

pid_t getpid(void) {
  pid_t ret;
  asm volatile("li a7, %1\n\t"
     148:	0ac00893          	li	a7,172
     14c:	00000073          	ecall
     150:	00050793          	mv	a5,a0
               "mv %0, a0\n\t"
               : "=r"(ret)
               : "i"(__NR_getpid)
               : "a0", "a7", "memory");
  return ret;
}
     154:	0007851b          	sext.w	a0,a5
     158:	00008067          	ret

000000000000015c <write>:

ssize_t write(int fd, const void *buf, size_t count) {
     15c:	00050793          	mv	a5,a0
     160:	00058713          	mv	a4,a1
     164:	00060693          	mv	a3,a2
  ssize_t ret;
  asm volatile("li a7, %1\n\t"
     168:	04000893          	li	a7,64
     16c:	00078513          	mv	a0,a5
     170:	00070593          	mv	a1,a4
     174:	00068613          	mv	a2,a3
     178:	00000073          	ecall
     17c:	00050793          	mv	a5,a0
               : "=r" (ret)
               : "i"(__NR_write), "r" (fd), "r" (buf), "r" (count)
               : "a0", "a1", "a2", "a7", "memory"
  );
  return ret;
}
     180:	00078513          	mv	a0,a5
     184:	00008067          	ret

0000000000000188 <srand>:
#include <stdint.h>

static uint64_t seed;

void srand(unsigned s) {
  seed = s - 1;
     188:	fff5051b          	addiw	a0,a0,-1
     18c:	02051513          	slli	a0,a0,0x20
     190:	02055513          	srli	a0,a0,0x20
     194:	00001797          	auipc	a5,0x1
     198:	4aa7ba23          	sd	a0,1204(a5) # 1648 <seed>
}
     19c:	00008067          	ret

00000000000001a0 <rand>:

int rand(void) {
  seed = 6364136223846793005ULL * seed + 1;
     1a0:	00001617          	auipc	a2,0x1
     1a4:	4a860613          	addi	a2,a2,1192 # 1648 <seed>
     1a8:	00063783          	ld	a5,0(a2)
     1ac:	00479693          	slli	a3,a5,0x4
     1b0:	40f686b3          	sub	a3,a3,a5
     1b4:	00669713          	slli	a4,a3,0x6
     1b8:	40d70733          	sub	a4,a4,a3
     1bc:	00771693          	slli	a3,a4,0x7
     1c0:	00d70733          	add	a4,a4,a3
     1c4:	00271693          	slli	a3,a4,0x2
     1c8:	00f68733          	add	a4,a3,a5
     1cc:	00671693          	slli	a3,a4,0x6
     1d0:	40e68733          	sub	a4,a3,a4
     1d4:	00771693          	slli	a3,a4,0x7
     1d8:	00f686b3          	add	a3,a3,a5
     1dc:	00269713          	slli	a4,a3,0x2
     1e0:	00f70733          	add	a4,a4,a5
     1e4:	00371693          	slli	a3,a4,0x3
     1e8:	40e686b3          	sub	a3,a3,a4
     1ec:	00369713          	slli	a4,a3,0x3
     1f0:	40d70733          	sub	a4,a4,a3
     1f4:	00671693          	slli	a3,a4,0x6
     1f8:	40e686b3          	sub	a3,a3,a4
     1fc:	00269713          	slli	a4,a3,0x2
     200:	40f70733          	sub	a4,a4,a5
     204:	00771693          	slli	a3,a4,0x7
     208:	40f686b3          	sub	a3,a3,a5
     20c:	00269713          	slli	a4,a3,0x2
     210:	00f70733          	add	a4,a4,a5
     214:	00271693          	slli	a3,a4,0x2
     218:	40f686b3          	sub	a3,a3,a5
     21c:	00269713          	slli	a4,a3,0x2
     220:	40f70733          	sub	a4,a4,a5
     224:	00271513          	slli	a0,a4,0x2
     228:	00f50533          	add	a0,a0,a5
     22c:	00150513          	addi	a0,a0,1
     230:	00a63023          	sd	a0,0(a2)
  return seed >> 33;
}
     234:	02155513          	srli	a0,a0,0x21
     238:	00008067          	ret

000000000000023c <memset>:
#include <string.h>

void *memset(void *restrict dst, int c, size_t n) {
    unsigned char *p = dst;
    unsigned char val = (unsigned char)c;
     23c:	0ff5f593          	zext.b	a1,a1
    unsigned char *p = dst;
     240:	00050713          	mv	a4,a0
    for (size_t i = 0; i < n; i++) {
     244:	00000793          	li	a5,0
     248:	0100006f          	j	258 <memset+0x1c>
        *p++ = val;
     24c:	00b70023          	sb	a1,0(a4)
    for (size_t i = 0; i < n; i++) {
     250:	00178793          	addi	a5,a5,1
        *p++ = val;
     254:	00170713          	addi	a4,a4,1
    for (size_t i = 0; i < n; i++) {
     258:	fec7eae3          	bltu	a5,a2,24c <memset+0x10>
    }
    return dst;
}
     25c:	00008067          	ret

0000000000000260 <memcpy>:

void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
    unsigned char *p_dst = dst;
     260:	00050713          	mv	a4,a0
    const unsigned char *p_src = src;
    for (size_t i = 0; i < n; i++) {
     264:	00000793          	li	a5,0
     268:	0180006f          	j	280 <memcpy+0x20>
        *p_dst++ = *p_src++;
     26c:	0005c683          	lbu	a3,0(a1)
     270:	00d70023          	sb	a3,0(a4)
    for (size_t i = 0; i < n; i++) {
     274:	00178793          	addi	a5,a5,1
        *p_dst++ = *p_src++;
     278:	00158593          	addi	a1,a1,1
     27c:	00170713          	addi	a4,a4,1
    for (size_t i = 0; i < n; i++) {
     280:	fec7e6e3          	bltu	a5,a2,26c <memcpy+0xc>
    }
    return dst;
}
     284:	00008067          	ret

0000000000000288 <strnlen>:

size_t strnlen(const char *restrict s, size_t maxlen) {
     288:	00050713          	mv	a4,a0
    size_t len = 0;
     28c:	00000513          	li	a0,0
    while (len < maxlen && s[len] != '\0') {
     290:	0080006f          	j	298 <strnlen+0x10>
        len++;
     294:	00150513          	addi	a0,a0,1
    while (len < maxlen && s[len] != '\0') {
     298:	00b57863          	bgeu	a0,a1,2a8 <strnlen+0x20>
     29c:	00a707b3          	add	a5,a4,a0
     2a0:	0007c783          	lbu	a5,0(a5)
     2a4:	fe0798e3          	bnez	a5,294 <strnlen+0xc>
    }
    return len;
}
     2a8:	00008067          	ret

00000000000002ac <clock>:
#include "time.h"

clock_t clock(void) {
    clock_t ret;
    asm volatile("rdtime %0" : "=r"(ret));
     2ac:	c0102573          	rdtime	a0
    // return ret / (TIMECLOCK / CLOCKS_PER_SEC); // bug
    return ret;
}
     2b0:	00008067          	ret

00000000000002b4 <pop_arg>:
  // long double f;
  void *p;
};

static void pop_arg(union arg *arg, int type, va_list *ap) {
  switch (type) {
     2b4:	ff85859b          	addiw	a1,a1,-8
     2b8:	00f00793          	li	a5,15
     2bc:	1ab7e063          	bltu	a5,a1,45c <pop_arg+0x1a8>
     2c0:	02059793          	slli	a5,a1,0x20
     2c4:	01e7d593          	srli	a1,a5,0x1e
     2c8:	00001717          	auipc	a4,0x1
     2cc:	e5870713          	addi	a4,a4,-424 # 1120 <__iob+0x78>
     2d0:	00e585b3          	add	a1,a1,a4
     2d4:	0005a783          	lw	a5,0(a1)
     2d8:	00e787b3          	add	a5,a5,a4
     2dc:	00078067          	jr	a5
    case PTR:
      arg->p = va_arg(*ap, void *);
     2e0:	00063783          	ld	a5,0(a2)
     2e4:	00878713          	addi	a4,a5,8
     2e8:	00e63023          	sd	a4,0(a2)
     2ec:	0007b783          	ld	a5,0(a5)
     2f0:	00f53023          	sd	a5,0(a0)
      break;
     2f4:	00008067          	ret
    case INT:
      arg->i = va_arg(*ap, int);
     2f8:	00063783          	ld	a5,0(a2)
     2fc:	00878713          	addi	a4,a5,8
     300:	00e63023          	sd	a4,0(a2)
     304:	0007a783          	lw	a5,0(a5)
     308:	00f53023          	sd	a5,0(a0)
      break;
     30c:	00008067          	ret
    case UINT:
      arg->i = va_arg(*ap, unsigned int);
     310:	00063783          	ld	a5,0(a2)
     314:	00878713          	addi	a4,a5,8
     318:	00e63023          	sd	a4,0(a2)
     31c:	0007e783          	lwu	a5,0(a5)
     320:	00f53023          	sd	a5,0(a0)
      break;
     324:	00008067          	ret
    case LONG:
      arg->i = va_arg(*ap, long);
     328:	00063783          	ld	a5,0(a2)
     32c:	00878713          	addi	a4,a5,8
     330:	00e63023          	sd	a4,0(a2)
     334:	0007b783          	ld	a5,0(a5)
     338:	00f53023          	sd	a5,0(a0)
      break;
     33c:	00008067          	ret
    case ULONG:
      arg->i = va_arg(*ap, unsigned long);
     340:	00063783          	ld	a5,0(a2)
     344:	00878713          	addi	a4,a5,8
     348:	00e63023          	sd	a4,0(a2)
     34c:	0007b783          	ld	a5,0(a5)
     350:	00f53023          	sd	a5,0(a0)
      break;
     354:	00008067          	ret
    case ULLONG:
      arg->i = va_arg(*ap, unsigned long long);
     358:	00063783          	ld	a5,0(a2)
     35c:	00878713          	addi	a4,a5,8
     360:	00e63023          	sd	a4,0(a2)
     364:	0007b783          	ld	a5,0(a5)
     368:	00f53023          	sd	a5,0(a0)
      break;
     36c:	00008067          	ret
    case SHORT:
      arg->i = (short)va_arg(*ap, int);
     370:	00063783          	ld	a5,0(a2)
     374:	00878713          	addi	a4,a5,8
     378:	00e63023          	sd	a4,0(a2)
     37c:	00079783          	lh	a5,0(a5)
     380:	00f53023          	sd	a5,0(a0)
      break;
     384:	00008067          	ret
    case USHORT:
      arg->i = (unsigned short)va_arg(*ap, int);
     388:	00063783          	ld	a5,0(a2)
     38c:	00878713          	addi	a4,a5,8
     390:	00e63023          	sd	a4,0(a2)
     394:	0007d783          	lhu	a5,0(a5)
     398:	00f53023          	sd	a5,0(a0)
      break;
     39c:	00008067          	ret
    case CHAR:
      arg->i = (signed char)va_arg(*ap, int);
     3a0:	00063783          	ld	a5,0(a2)
     3a4:	00878713          	addi	a4,a5,8
     3a8:	00e63023          	sd	a4,0(a2)
     3ac:	00078783          	lb	a5,0(a5)
     3b0:	00f53023          	sd	a5,0(a0)
      break;
     3b4:	00008067          	ret
    case UCHAR:
      arg->i = (unsigned char)va_arg(*ap, int);
     3b8:	00063783          	ld	a5,0(a2)
     3bc:	00878713          	addi	a4,a5,8
     3c0:	00e63023          	sd	a4,0(a2)
     3c4:	0007c783          	lbu	a5,0(a5)
     3c8:	00f53023          	sd	a5,0(a0)
      break;
     3cc:	00008067          	ret
    case LLONG:
      arg->i = va_arg(*ap, long long);
     3d0:	00063783          	ld	a5,0(a2)
     3d4:	00878713          	addi	a4,a5,8
     3d8:	00e63023          	sd	a4,0(a2)
     3dc:	0007b783          	ld	a5,0(a5)
     3e0:	00f53023          	sd	a5,0(a0)
      break;
     3e4:	00008067          	ret
    case SIZET:
      arg->i = va_arg(*ap, size_t);
     3e8:	00063783          	ld	a5,0(a2)
     3ec:	00878713          	addi	a4,a5,8
     3f0:	00e63023          	sd	a4,0(a2)
     3f4:	0007b783          	ld	a5,0(a5)
     3f8:	00f53023          	sd	a5,0(a0)
      break;
     3fc:	00008067          	ret
    case IMAX:
      arg->i = va_arg(*ap, intmax_t);
     400:	00063783          	ld	a5,0(a2)
     404:	00878713          	addi	a4,a5,8
     408:	00e63023          	sd	a4,0(a2)
     40c:	0007b783          	ld	a5,0(a5)
     410:	00f53023          	sd	a5,0(a0)
      break;
     414:	00008067          	ret
    case UMAX:
      arg->i = va_arg(*ap, uintmax_t);
     418:	00063783          	ld	a5,0(a2)
     41c:	00878713          	addi	a4,a5,8
     420:	00e63023          	sd	a4,0(a2)
     424:	0007b783          	ld	a5,0(a5)
     428:	00f53023          	sd	a5,0(a0)
      break;
     42c:	00008067          	ret
    case PDIFF:
      arg->i = va_arg(*ap, ptrdiff_t);
     430:	00063783          	ld	a5,0(a2)
     434:	00878713          	addi	a4,a5,8
     438:	00e63023          	sd	a4,0(a2)
     43c:	0007b783          	ld	a5,0(a5)
     440:	00f53023          	sd	a5,0(a0)
      break;
     444:	00008067          	ret
    case UIPTR:
      arg->i = (uintptr_t)va_arg(*ap, void *);
     448:	00063783          	ld	a5,0(a2)
     44c:	00878713          	addi	a4,a5,8
     450:	00e63023          	sd	a4,0(a2)
     454:	0007b783          	ld	a5,0(a5)
     458:	00f53023          	sd	a5,0(a0)
      //   arg->f = va_arg(*ap, double);
      //   break;
      // case LDBL:
      //   arg->f = va_arg(*ap, long double);
  }
}
     45c:	00008067          	ret

0000000000000460 <out>:

static void out(FILE *f, const char *s, size_t l) {
     460:	ff010113          	addi	sp,sp,-16
     464:	00113423          	sd	ra,8(sp)
  f->write(f, s, l);
     468:	00853783          	ld	a5,8(a0)
     46c:	000780e7          	jalr	a5
}
     470:	00813083          	ld	ra,8(sp)
     474:	01010113          	addi	sp,sp,16
     478:	00008067          	ret

000000000000047c <fmt_x>:
  out(f, pad, l);
}

static const char xdigits[16] = {"0123456789ABCDEF"};

static char *fmt_x(uintmax_t x, char *s, int lower) {
     47c:	00050793          	mv	a5,a0
     480:	00058513          	mv	a0,a1
  for (; x; x >>= 4)
     484:	0280006f          	j	4ac <fmt_x+0x30>
    *--s = xdigits[(x & 15)] | lower;
     488:	00f7f693          	andi	a3,a5,15
     48c:	00001717          	auipc	a4,0x1
     490:	d7470713          	addi	a4,a4,-652 # 1200 <xdigits>
     494:	00d70733          	add	a4,a4,a3
     498:	00074703          	lbu	a4,0(a4)
     49c:	fff50513          	addi	a0,a0,-1
     4a0:	00c76733          	or	a4,a4,a2
     4a4:	00e50023          	sb	a4,0(a0)
  for (; x; x >>= 4)
     4a8:	0047d793          	srli	a5,a5,0x4
     4ac:	fc079ee3          	bnez	a5,488 <fmt_x+0xc>
  return s;
}
     4b0:	00008067          	ret

00000000000004b4 <fmt_o>:

static char *fmt_o(uintmax_t x, char *s) {
     4b4:	00050793          	mv	a5,a0
     4b8:	00058513          	mv	a0,a1
  for (; x; x >>= 3)
     4bc:	0180006f          	j	4d4 <fmt_o+0x20>
    *--s = '0' + (x & 7);
     4c0:	0077f713          	andi	a4,a5,7
     4c4:	fff50513          	addi	a0,a0,-1
     4c8:	03070713          	addi	a4,a4,48
     4cc:	00e50023          	sb	a4,0(a0)
  for (; x; x >>= 3)
     4d0:	0037d793          	srli	a5,a5,0x3
     4d4:	fe0796e3          	bnez	a5,4c0 <fmt_o+0xc>
  return s;
}
     4d8:	00008067          	ret

00000000000004dc <fmt_u>:

static char *fmt_u(uintmax_t x, char *s) {
     4dc:	fe010113          	addi	sp,sp,-32
     4e0:	00113c23          	sd	ra,24(sp)
     4e4:	00813823          	sd	s0,16(sp)
     4e8:	00913423          	sd	s1,8(sp)
     4ec:	00050413          	mv	s0,a0
     4f0:	00058493          	mv	s1,a1
  unsigned long y;
  for (; x > ULONG_MAX; x /= 10)
    *--s = '0' + x % 10;
  for (y = x; y; y /= 10)
     4f4:	02c0006f          	j	520 <fmt_u+0x44>
    *--s = '0' + y % 10;
     4f8:	00a00593          	li	a1,10
     4fc:	00040513          	mv	a0,s0
     500:	315000ef          	jal	1014 <__umoddi3>
     504:	fff48493          	addi	s1,s1,-1
     508:	0305051b          	addiw	a0,a0,48
     50c:	00a48023          	sb	a0,0(s1)
  for (y = x; y; y /= 10)
     510:	00a00593          	li	a1,10
     514:	00040513          	mv	a0,s0
     518:	2b5000ef          	jal	fcc <__hidden___udivdi3>
     51c:	00050413          	mv	s0,a0
     520:	fc041ce3          	bnez	s0,4f8 <fmt_u+0x1c>
  return s;
}
     524:	00048513          	mv	a0,s1
     528:	01813083          	ld	ra,24(sp)
     52c:	01013403          	ld	s0,16(sp)
     530:	00813483          	ld	s1,8(sp)
     534:	02010113          	addi	sp,sp,32
     538:	00008067          	ret

000000000000053c <getint>:

static int getint(char **s) {
     53c:	00050813          	mv	a6,a0
  int i;
  for (i = 0; isdigit(**s); (*s)++) {
     540:	00000513          	li	a0,0
     544:	0100006f          	j	554 <getint+0x18>
    if (i > INT_MAX / 10 || **s - '0' > INT_MAX - 10 * i)
      i = -1;
     548:	fff00513          	li	a0,-1
  for (i = 0; isdigit(**s); (*s)++) {
     54c:	00178793          	addi	a5,a5,1
     550:	00f83023          	sd	a5,0(a6)
     554:	00083783          	ld	a5,0(a6)
     558:	0007c683          	lbu	a3,0(a5)
static inline int iscntrl(int c) {
  return (c >= 0 && c <= 0x1f) || c == 0x7f;
}

static inline int isdigit(int c) {
  return c >= '0' && c <= '9';
     55c:	fd06861b          	addiw	a2,a3,-48
     560:	00900713          	li	a4,9
     564:	04c76263          	bltu	a4,a2,5a8 <getint+0x6c>
    if (i > INT_MAX / 10 || **s - '0' > INT_MAX - 10 * i)
     568:	0cccd737          	lui	a4,0xcccd
     56c:	ccc70713          	addi	a4,a4,-820 # ccccccc <seed+0xcccb684>
     570:	fca74ce3          	blt	a4,a0,548 <getint+0xc>
     574:	fd06869b          	addiw	a3,a3,-48
     578:	0025161b          	slliw	a2,a0,0x2
     57c:	00a6063b          	addw	a2,a2,a0
     580:	0016161b          	slliw	a2,a2,0x1
     584:	40c0073b          	negw	a4,a2
     588:	800005b7          	lui	a1,0x80000
     58c:	fff5859b          	addiw	a1,a1,-1 # 7fffffff <seed+0x7fffe9b7>
     590:	40c5863b          	subw	a2,a1,a2
     594:	00d64663          	blt	a2,a3,5a0 <getint+0x64>
    else
      i = 10 * i + (**s - '0');
     598:	40e6853b          	subw	a0,a3,a4
     59c:	fb1ff06f          	j	54c <getint+0x10>
      i = -1;
     5a0:	fff00513          	li	a0,-1
     5a4:	fa9ff06f          	j	54c <getint+0x10>
  }
  return i;
}
     5a8:	00008067          	ret

00000000000005ac <pad>:
  if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w)
     5ac:	000127b7          	lui	a5,0x12
     5b0:	00f77733          	and	a4,a4,a5
     5b4:	08071063          	bnez	a4,634 <pad+0x88>
static void pad(FILE *f, char c, size_t w, size_t l, int fl) {
     5b8:	ee010113          	addi	sp,sp,-288
     5bc:	10113c23          	sd	ra,280(sp)
     5c0:	10913423          	sd	s1,264(sp)
     5c4:	00050493          	mv	s1,a0
  if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w)
     5c8:	00c6ea63          	bltu	a3,a2,5dc <pad+0x30>
}
     5cc:	11813083          	ld	ra,280(sp)
     5d0:	10813483          	ld	s1,264(sp)
     5d4:	12010113          	addi	sp,sp,288
     5d8:	00008067          	ret
     5dc:	10813823          	sd	s0,272(sp)
  l = w - l;
     5e0:	40d60433          	sub	s0,a2,a3
  memset(pad, c, l > sizeof pad ? sizeof pad : l);
     5e4:	00040613          	mv	a2,s0
     5e8:	10000793          	li	a5,256
     5ec:	0087f463          	bgeu	a5,s0,5f4 <pad+0x48>
     5f0:	00078613          	mv	a2,a5
     5f4:	00010513          	mv	a0,sp
     5f8:	c45ff0ef          	jal	23c <memset>
  for (; l >= sizeof pad; l -= sizeof pad)
     5fc:	0180006f          	j	614 <pad+0x68>
    out(f, pad, sizeof pad);
     600:	10000613          	li	a2,256
     604:	00010593          	mv	a1,sp
     608:	00048513          	mv	a0,s1
     60c:	e55ff0ef          	jal	460 <out>
  for (; l >= sizeof pad; l -= sizeof pad)
     610:	f0040413          	addi	s0,s0,-256
     614:	0ff00793          	li	a5,255
     618:	fe87e4e3          	bltu	a5,s0,600 <pad+0x54>
  out(f, pad, l);
     61c:	00040613          	mv	a2,s0
     620:	00010593          	mv	a1,sp
     624:	00048513          	mv	a0,s1
     628:	e39ff0ef          	jal	460 <out>
     62c:	11013403          	ld	s0,272(sp)
     630:	f9dff06f          	j	5cc <pad+0x20>
     634:	00008067          	ret

0000000000000638 <printf_core>:

// theoretically you can implement all other *printf functions using this one...
static int printf_core(FILE *f, const char *fmt, va_list *ap, union arg *nl_arg, int *nl_type) {
     638:	f4010113          	addi	sp,sp,-192
     63c:	0a113c23          	sd	ra,184(sp)
     640:	0a813823          	sd	s0,176(sp)
     644:	0a913423          	sd	s1,168(sp)
     648:	0b213023          	sd	s2,160(sp)
     64c:	09313c23          	sd	s3,152(sp)
     650:	09413823          	sd	s4,144(sp)
     654:	09513423          	sd	s5,136(sp)
     658:	09613023          	sd	s6,128(sp)
     65c:	07713c23          	sd	s7,120(sp)
     660:	07813823          	sd	s8,112(sp)
     664:	07913423          	sd	s9,104(sp)
     668:	07a13023          	sd	s10,96(sp)
     66c:	05b13c23          	sd	s11,88(sp)
     670:	00050b13          	mv	s6,a0
     674:	00060d93          	mv	s11,a2
     678:	00d13823          	sd	a3,16(sp)
     67c:	00e13c23          	sd	a4,24(sp)
  char *a, *z, *s = (char *)fmt;
     680:	04b13423          	sd	a1,72(sp)
  unsigned l10n = 0, fl;
  int w, p, xp;
  union arg arg;
  int argpos;
  unsigned st, ps;
  int cnt = 0, l = 0;
     684:	00000413          	li	s0,0
     688:	00000a93          	li	s5,0
  unsigned l10n = 0, fl;
     68c:	00013023          	sd	zero,0(sp)
     690:	0780006f          	j	708 <printf_core+0xd0>
    cnt += l;
    if (!*s)
      break;

    /* Handle literal text and %% format specifiers */
    for (a = s; *s && *s != '%'; s++)
     694:	00140413          	addi	s0,s0,1
     698:	04813423          	sd	s0,72(sp)
     69c:	04813403          	ld	s0,72(sp)
     6a0:	00044783          	lbu	a5,0(s0)
     6a4:	00078663          	beqz	a5,6b0 <printf_core+0x78>
     6a8:	02500713          	li	a4,37
     6ac:	fee794e3          	bne	a5,a4,694 <printf_core+0x5c>
      ;
    for (z = s; s[0] == '%' && s[1] == '%'; z++, s += 2)
     6b0:	04813783          	ld	a5,72(sp)
     6b4:	0007c683          	lbu	a3,0(a5) # 12000 <seed+0x109b8>
     6b8:	02500713          	li	a4,37
     6bc:	00e69e63          	bne	a3,a4,6d8 <printf_core+0xa0>
     6c0:	0017c683          	lbu	a3,1(a5)
     6c4:	00e69a63          	bne	a3,a4,6d8 <printf_core+0xa0>
     6c8:	00140413          	addi	s0,s0,1
     6cc:	00278793          	addi	a5,a5,2
     6d0:	04f13423          	sd	a5,72(sp)
     6d4:	fddff06f          	j	6b0 <printf_core+0x78>
      ;
    if (z - a > INT_MAX - cnt)
     6d8:	41440433          	sub	s0,s0,s4
     6dc:	800009b7          	lui	s3,0x80000
     6e0:	fff9899b          	addiw	s3,s3,-1 # 7fffffff <seed+0x7fffe9b7>
     6e4:	417989bb          	subw	s3,s3,s7
     6e8:	7889c063          	blt	s3,s0,e68 <printf_core+0x830>
      goto overflow;
    l = z - a;
     6ec:	0004041b          	sext.w	s0,s0
    if (f)
     6f0:	000b0a63          	beqz	s6,704 <printf_core+0xcc>
      out(f, a, l);
     6f4:	00040613          	mv	a2,s0
     6f8:	000a0593          	mv	a1,s4
     6fc:	000b0513          	mv	a0,s6
     700:	d61ff0ef          	jal	460 <out>
    if (l)
     704:	02040e63          	beqz	s0,740 <printf_core+0x108>
    if (l > INT_MAX - cnt)
     708:	800007b7          	lui	a5,0x80000
     70c:	fff7879b          	addiw	a5,a5,-1 # 7fffffff <seed+0x7fffe9b7>
     710:	415787bb          	subw	a5,a5,s5
     714:	7487c663          	blt	a5,s0,e60 <printf_core+0x828>
    cnt += l;
     718:	008a8bbb          	addw	s7,s5,s0
     71c:	000b8a93          	mv	s5,s7
    if (!*s)
     720:	04813a03          	ld	s4,72(sp)
     724:	000a4783          	lbu	a5,0(s4)
     728:	f6079ae3          	bnez	a5,69c <printf_core+0x64>
    pad(f, ' ', w, pl + p, fl ^ LEFT_ADJ);

    l = w;
  }

  if (f)
     72c:	760b1063          	bnez	s6,e8c <printf_core+0x854>
    return cnt;
  if (!l10n)
     730:	00013783          	ld	a5,0(sp)
     734:	7c078463          	beqz	a5,efc <printf_core+0x8c4>
    return 0;

  for (i = 1; i <= NL_ARGMAX && nl_type[i]; i++)
     738:	00100413          	li	s0,1
     73c:	6d40006f          	j	e10 <printf_core+0x7d8>
    if (isdigit(s[1]) && s[2] == '$') {
     740:	04813783          	ld	a5,72(sp)
     744:	0017cc83          	lbu	s9,1(a5)
     748:	fd0c869b          	addiw	a3,s9,-48
     74c:	00900713          	li	a4,9
     750:	00d76863          	bltu	a4,a3,760 <printf_core+0x128>
     754:	0027c683          	lbu	a3,2(a5)
     758:	02400713          	li	a4,36
     75c:	00e68c63          	beq	a3,a4,774 <printf_core+0x13c>
      s++;
     760:	00178793          	addi	a5,a5,1
     764:	04f13423          	sd	a5,72(sp)
      argpos = -1;
     768:	fff00c93          	li	s9,-1
    for (fl = 0; (unsigned)(*s - ' ') < 32 && (FLAGMASK & (1U << (*s - ' '))); s++)
     76c:	00000493          	li	s1,0
     770:	0300006f          	j	7a0 <printf_core+0x168>
      argpos = s[1] - '0';
     774:	fd0c8c9b          	addiw	s9,s9,-48
      s += 3;
     778:	00378793          	addi	a5,a5,3
     77c:	04f13423          	sd	a5,72(sp)
      l10n = 1;
     780:	00100793          	li	a5,1
     784:	00f13023          	sd	a5,0(sp)
      s += 3;
     788:	fe5ff06f          	j	76c <printf_core+0x134>
      fl |= 1U << (*s - ' ');
     78c:	00100793          	li	a5,1
     790:	00d797bb          	sllw	a5,a5,a3
     794:	0097e4b3          	or	s1,a5,s1
    for (fl = 0; (unsigned)(*s - ' ') < 32 && (FLAGMASK & (1U << (*s - ' '))); s++)
     798:	00170713          	addi	a4,a4,1
     79c:	04e13423          	sd	a4,72(sp)
     7a0:	04813703          	ld	a4,72(sp)
     7a4:	00074603          	lbu	a2,0(a4)
     7a8:	fe06069b          	addiw	a3,a2,-32
     7ac:	01f00793          	li	a5,31
     7b0:	00d7ec63          	bltu	a5,a3,7c8 <printf_core+0x190>
     7b4:	000137b7          	lui	a5,0x13
     7b8:	8097879b          	addiw	a5,a5,-2039 # 12809 <seed+0x111c1>
     7bc:	00d7d7bb          	srlw	a5,a5,a3
     7c0:	0017f793          	andi	a5,a5,1
     7c4:	fc0794e3          	bnez	a5,78c <printf_core+0x154>
    if (*s == '*') {
     7c8:	02a00793          	li	a5,42
     7cc:	0af61a63          	bne	a2,a5,880 <printf_core+0x248>
      if (isdigit(s[1]) && s[2] == '$') {
     7d0:	00174783          	lbu	a5,1(a4)
     7d4:	fd07861b          	addiw	a2,a5,-48
     7d8:	00900693          	li	a3,9
     7dc:	00c6e863          	bltu	a3,a2,7ec <printf_core+0x1b4>
     7e0:	00274683          	lbu	a3,2(a4)
     7e4:	02400713          	li	a4,36
     7e8:	04e68063          	beq	a3,a4,828 <printf_core+0x1f0>
      } else if (!l10n) {
     7ec:	00013783          	ld	a5,0(sp)
     7f0:	68079063          	bnez	a5,e70 <printf_core+0x838>
        w = f ? va_arg(*ap, int) : 0;
     7f4:	080b0263          	beqz	s6,878 <printf_core+0x240>
     7f8:	000db783          	ld	a5,0(s11)
     7fc:	00878713          	addi	a4,a5,8
     800:	00edb023          	sd	a4,0(s11)
     804:	0007ac03          	lw	s8,0(a5)
        s++;
     808:	04813783          	ld	a5,72(sp)
     80c:	00178793          	addi	a5,a5,1
     810:	04f13423          	sd	a5,72(sp)
      if (w < 0)
     814:	060c5e63          	bgez	s8,890 <printf_core+0x258>
        fl |= LEFT_ADJ, w = -w;
     818:	000027b7          	lui	a5,0x2
     81c:	00f4e4b3          	or	s1,s1,a5
     820:	41800c3b          	negw	s8,s8
     824:	06c0006f          	j	890 <printf_core+0x258>
        if (!f)
     828:	020b0863          	beqz	s6,858 <printf_core+0x220>
          w = nl_arg[s[1] - '0'].i;
     82c:	00379793          	slli	a5,a5,0x3
     830:	e8078793          	addi	a5,a5,-384 # 1e80 <seed+0x838>
     834:	01013703          	ld	a4,16(sp)
     838:	00f707b3          	add	a5,a4,a5
     83c:	0007ac03          	lw	s8,0(a5)
        s += 3;
     840:	04813783          	ld	a5,72(sp)
     844:	00378793          	addi	a5,a5,3
     848:	04f13423          	sd	a5,72(sp)
        l10n = 1;
     84c:	00100793          	li	a5,1
     850:	00f13023          	sd	a5,0(sp)
        s += 3;
     854:	fc1ff06f          	j	814 <printf_core+0x1dc>
          nl_type[s[1] - '0'] = INT, w = 0;
     858:	00279793          	slli	a5,a5,0x2
     85c:	f4078793          	addi	a5,a5,-192
     860:	01813703          	ld	a4,24(sp)
     864:	00f707b3          	add	a5,a4,a5
     868:	00900713          	li	a4,9
     86c:	00e7a023          	sw	a4,0(a5)
     870:	00040c13          	mv	s8,s0
     874:	fcdff06f          	j	840 <printf_core+0x208>
        w = f ? va_arg(*ap, int) : 0;
     878:	00040c13          	mv	s8,s0
     87c:	f8dff06f          	j	808 <printf_core+0x1d0>
    } else if ((w = getint(&s)) < 0)
     880:	04810513          	addi	a0,sp,72
     884:	cb9ff0ef          	jal	53c <getint>
     888:	00050c13          	mv	s8,a0
     88c:	5e054663          	bltz	a0,e78 <printf_core+0x840>
    if (*s == '.' && s[1] == '*') {
     890:	04813783          	ld	a5,72(sp)
     894:	0007c703          	lbu	a4,0(a5)
     898:	02e00693          	li	a3,46
     89c:	0ad71a63          	bne	a4,a3,950 <printf_core+0x318>
     8a0:	0017c603          	lbu	a2,1(a5)
     8a4:	02a00693          	li	a3,42
     8a8:	0ad61463          	bne	a2,a3,950 <printf_core+0x318>
      if (isdigit(s[2]) && s[3] == '$') {
     8ac:	0027c703          	lbu	a4,2(a5)
     8b0:	fd07061b          	addiw	a2,a4,-48
     8b4:	00900693          	li	a3,9
     8b8:	00c6e863          	bltu	a3,a2,8c8 <printf_core+0x290>
     8bc:	0037c683          	lbu	a3,3(a5)
     8c0:	02400793          	li	a5,36
     8c4:	02f68e63          	beq	a3,a5,900 <printf_core+0x2c8>
      } else if (!l10n) {
     8c8:	00013783          	ld	a5,0(sp)
     8cc:	5a079a63          	bnez	a5,e80 <printf_core+0x848>
        p = f ? va_arg(*ap, int) : 0;
     8d0:	060b0c63          	beqz	s6,948 <printf_core+0x310>
     8d4:	000db783          	ld	a5,0(s11)
     8d8:	00878713          	addi	a4,a5,8
     8dc:	00edb023          	sd	a4,0(s11)
     8e0:	0007ad03          	lw	s10,0(a5)
        s += 2;
     8e4:	04813783          	ld	a5,72(sp)
     8e8:	00278793          	addi	a5,a5,2
     8ec:	04f13423          	sd	a5,72(sp)
      xp = (p >= 0);
     8f0:	fffd4793          	not	a5,s10
     8f4:	01f7d79b          	srliw	a5,a5,0x1f
     8f8:	00f13423          	sd	a5,8(sp)
     8fc:	0640006f          	j	960 <printf_core+0x328>
        if (!f)
     900:	020b0463          	beqz	s6,928 <printf_core+0x2f0>
          p = nl_arg[s[2] - '0'].i;
     904:	00371793          	slli	a5,a4,0x3
     908:	e8078793          	addi	a5,a5,-384
     90c:	01013703          	ld	a4,16(sp)
     910:	00f707b3          	add	a5,a4,a5
     914:	0007ad03          	lw	s10,0(a5)
        s += 4;
     918:	04813783          	ld	a5,72(sp)
     91c:	00478793          	addi	a5,a5,4
     920:	04f13423          	sd	a5,72(sp)
     924:	fcdff06f          	j	8f0 <printf_core+0x2b8>
          nl_type[s[2] - '0'] = INT, p = 0;
     928:	00271793          	slli	a5,a4,0x2
     92c:	f4078793          	addi	a5,a5,-192
     930:	01813703          	ld	a4,24(sp)
     934:	00f707b3          	add	a5,a4,a5
     938:	00900713          	li	a4,9
     93c:	00e7a023          	sw	a4,0(a5)
     940:	00040d13          	mv	s10,s0
     944:	fd5ff06f          	j	918 <printf_core+0x2e0>
        p = f ? va_arg(*ap, int) : 0;
     948:	00040d13          	mv	s10,s0
     94c:	f99ff06f          	j	8e4 <printf_core+0x2ac>
    } else if (*s == '.') {
     950:	02e00693          	li	a3,46
     954:	00d70a63          	beq	a4,a3,968 <printf_core+0x330>
      xp = 0;
     958:	00813423          	sd	s0,8(sp)
      p = -1;
     95c:	fff00d13          	li	s10,-1
    st = 0;
     960:	00000913          	li	s2,0
     964:	0280006f          	j	98c <printf_core+0x354>
      s++;
     968:	00178793          	addi	a5,a5,1
     96c:	04f13423          	sd	a5,72(sp)
      p = getint(&s);
     970:	04810513          	addi	a0,sp,72
     974:	bc9ff0ef          	jal	53c <getint>
     978:	00050d13          	mv	s10,a0
      xp = 1;
     97c:	00100793          	li	a5,1
     980:	00f13423          	sd	a5,8(sp)
     984:	fddff06f          	j	960 <printf_core+0x328>
      st = states[st] S(*s++);
     988:	00078913          	mv	s2,a5
      if (OOB(*s))
     98c:	04813703          	ld	a4,72(sp)
     990:	00074783          	lbu	a5,0(a4)
     994:	fbf7879b          	addiw	a5,a5,-65
     998:	03900693          	li	a3,57
     99c:	4ef6e663          	bltu	a3,a5,e88 <printf_core+0x850>
      st = states[st] S(*s++);
     9a0:	00170793          	addi	a5,a4,1
     9a4:	04f13423          	sd	a5,72(sp)
     9a8:	00074683          	lbu	a3,0(a4)
     9ac:	fbf6869b          	addiw	a3,a3,-65
     9b0:	02091713          	slli	a4,s2,0x20
     9b4:	02075713          	srli	a4,a4,0x20
     9b8:	00371793          	slli	a5,a4,0x3
     9bc:	40e787b3          	sub	a5,a5,a4
     9c0:	00279793          	slli	a5,a5,0x2
     9c4:	00e787b3          	add	a5,a5,a4
     9c8:	00179793          	slli	a5,a5,0x1
     9cc:	00001717          	auipc	a4,0x1
     9d0:	84470713          	addi	a4,a4,-1980 # 1210 <states>
     9d4:	00f707b3          	add	a5,a4,a5
     9d8:	00d787b3          	add	a5,a5,a3
     9dc:	0007c583          	lbu	a1,0(a5)
     9e0:	0005879b          	sext.w	a5,a1
    } while (st - 1 < STOP);
     9e4:	fff5869b          	addiw	a3,a1,-1
     9e8:	00600713          	li	a4,6
     9ec:	f8d77ee3          	bgeu	a4,a3,988 <printf_core+0x350>
    if (!st)
     9f0:	4c078e63          	beqz	a5,ecc <printf_core+0x894>
    if (st == NOARG) {
     9f4:	01800713          	li	a4,24
     9f8:	02e78263          	beq	a5,a4,a1c <printf_core+0x3e4>
      if (argpos >= 0) {
     9fc:	080cc663          	bltz	s9,a88 <printf_core+0x450>
        if (!f)
     a00:	060b0a63          	beqz	s6,a74 <printf_core+0x43c>
          arg = nl_arg[argpos];
     a04:	003c9793          	slli	a5,s9,0x3
     a08:	01013703          	ld	a4,16(sp)
     a0c:	00f707b3          	add	a5,a4,a5
     a10:	0007b783          	ld	a5,0(a5)
     a14:	04f13023          	sd	a5,64(sp)
     a18:	0080006f          	j	a20 <printf_core+0x3e8>
      if (argpos >= 0)
     a1c:	4a0cdc63          	bgez	s9,ed4 <printf_core+0x89c>
    if (!f)
     a20:	ce0b04e3          	beqz	s6,708 <printf_core+0xd0>
    t = s[-1];
     a24:	04813783          	ld	a5,72(sp)
     a28:	fff7c783          	lbu	a5,-1(a5)
     a2c:	00078c9b          	sext.w	s9,a5
    if (fl & LEFT_ADJ)
     a30:	00002737          	lui	a4,0x2
     a34:	00e4f733          	and	a4,s1,a4
     a38:	00070863          	beqz	a4,a48 <printf_core+0x410>
      fl &= ~ZERO_PAD;
     a3c:	ffff0737          	lui	a4,0xffff0
     a40:	fff70713          	addi	a4,a4,-1 # fffffffffffeffff <seed+0xfffffffffffee9b7>
     a44:	00e4f4b3          	and	s1,s1,a4
    switch (t) {
     a48:	fa87879b          	addiw	a5,a5,-88
     a4c:	0ff7f693          	zext.b	a3,a5
     a50:	02000713          	li	a4,32
     a54:	2cd76863          	bltu	a4,a3,d24 <printf_core+0x6ec>
     a58:	00269793          	slli	a5,a3,0x2
     a5c:	00000717          	auipc	a4,0x0
     a60:	70470713          	addi	a4,a4,1796 # 1160 <__iob+0xb8>
     a64:	00e787b3          	add	a5,a5,a4
     a68:	0007a783          	lw	a5,0(a5)
     a6c:	00e787b3          	add	a5,a5,a4
     a70:	00078067          	jr	a5
          nl_type[argpos] = st;
     a74:	002c9793          	slli	a5,s9,0x2
     a78:	01813703          	ld	a4,24(sp)
     a7c:	00f707b3          	add	a5,a4,a5
     a80:	00b7a023          	sw	a1,0(a5)
     a84:	f9dff06f          	j	a20 <printf_core+0x3e8>
      } else if (f)
     a88:	440b0a63          	beqz	s6,edc <printf_core+0x8a4>
        pop_arg(&arg, st, ap);
     a8c:	000d8613          	mv	a2,s11
     a90:	04010513          	addi	a0,sp,64
     a94:	821ff0ef          	jal	2b4 <pop_arg>
     a98:	f89ff06f          	j	a20 <printf_core+0x3e8>
        switch (ps) {
     a9c:	00600793          	li	a5,6
     aa0:	c727e4e3          	bltu	a5,s2,708 <printf_core+0xd0>
     aa4:	00291793          	slli	a5,s2,0x2
     aa8:	00000717          	auipc	a4,0x0
     aac:	73c70713          	addi	a4,a4,1852 # 11e4 <__iob+0x13c>
     ab0:	00e787b3          	add	a5,a5,a4
     ab4:	0007a783          	lw	a5,0(a5)
     ab8:	00e787b3          	add	a5,a5,a4
     abc:	00078067          	jr	a5
            *(int *)arg.p = cnt;
     ac0:	04013783          	ld	a5,64(sp)
     ac4:	0177a023          	sw	s7,0(a5)
            break;
     ac8:	c41ff06f          	j	708 <printf_core+0xd0>
            *(long *)arg.p = cnt;
     acc:	04013783          	ld	a5,64(sp)
     ad0:	0177b023          	sd	s7,0(a5)
            break;
     ad4:	c35ff06f          	j	708 <printf_core+0xd0>
            *(long long *)arg.p = cnt;
     ad8:	04013783          	ld	a5,64(sp)
     adc:	0177b023          	sd	s7,0(a5)
            break;
     ae0:	c29ff06f          	j	708 <printf_core+0xd0>
            *(unsigned short *)arg.p = cnt;
     ae4:	04013783          	ld	a5,64(sp)
     ae8:	01779023          	sh	s7,0(a5)
            break;
     aec:	c1dff06f          	j	708 <printf_core+0xd0>
            *(unsigned char *)arg.p = cnt;
     af0:	04013783          	ld	a5,64(sp)
     af4:	01778023          	sb	s7,0(a5)
            break;
     af8:	c11ff06f          	j	708 <printf_core+0xd0>
            *(size_t *)arg.p = cnt;
     afc:	04013783          	ld	a5,64(sp)
     b00:	0177b023          	sd	s7,0(a5)
            break;
     b04:	c05ff06f          	j	708 <printf_core+0xd0>
            *(uintmax_t *)arg.p = cnt;
     b08:	04013783          	ld	a5,64(sp)
     b0c:	0177b023          	sd	s7,0(a5)
        continue;
     b10:	bf9ff06f          	j	708 <printf_core+0xd0>
        p = MAX((size_t)p, 2 * sizeof(void *));
     b14:	01000793          	li	a5,16
     b18:	00fd7463          	bgeu	s10,a5,b20 <printf_core+0x4e8>
     b1c:	00078d13          	mv	s10,a5
     b20:	000d0d1b          	sext.w	s10,s10
        fl |= ALT_FORM;
     b24:	0084e493          	ori	s1,s1,8
        t = 'x';
     b28:	07800c93          	li	s9,120
        a = fmt_x(arg.i, z, t & 32);
     b2c:	020cf613          	andi	a2,s9,32
     b30:	04010593          	addi	a1,sp,64
     b34:	04013503          	ld	a0,64(sp)
     b38:	945ff0ef          	jal	47c <fmt_x>
     b3c:	00050a13          	mv	s4,a0
        if (arg.i && (fl & ALT_FORM))
     b40:	04013783          	ld	a5,64(sp)
     b44:	10078e63          	beqz	a5,c60 <printf_core+0x628>
     b48:	0084f793          	andi	a5,s1,8
     b4c:	12078063          	beqz	a5,c6c <printf_core+0x634>
          prefix += (t >> 4), pl = 2;
     b50:	404cdc9b          	sraiw	s9,s9,0x4
     b54:	00000797          	auipc	a5,0x0
     b58:	5b478793          	addi	a5,a5,1460 # 1108 <__iob+0x60>
     b5c:	01978cb3          	add	s9,a5,s9
     b60:	00200413          	li	s0,2
     b64:	0980006f          	j	bfc <printf_core+0x5c4>
            a = fmt_o(arg.i, z);
     b68:	04010593          	addi	a1,sp,64
     b6c:	04013503          	ld	a0,64(sp)
     b70:	945ff0ef          	jal	4b4 <fmt_o>
     b74:	00050a13          	mv	s4,a0
            if ((fl & ALT_FORM) && p < z - a + 1)
     b78:	0084f793          	andi	a5,s1,8
     b7c:	0e078e63          	beqz	a5,c78 <printf_core+0x640>
     b80:	04010793          	addi	a5,sp,64
     b84:	40a787b3          	sub	a5,a5,a0
     b88:	0fa7ce63          	blt	a5,s10,c84 <printf_core+0x64c>
              p = z - a + 1;
     b8c:	00178d1b          	addiw	s10,a5,1
    prefix = "-+   0X0x";
     b90:	00000c97          	auipc	s9,0x0
     b94:	578c8c93          	addi	s9,s9,1400 # 1108 <__iob+0x60>
     b98:	0640006f          	j	bfc <printf_core+0x5c4>
            if (arg.i > INTMAX_MAX) {
     b9c:	04013783          	ld	a5,64(sp)
     ba0:	0207c663          	bltz	a5,bcc <printf_core+0x594>
            } else if (fl & MARK_POS) {
     ba4:	000017b7          	lui	a5,0x1
     ba8:	80078793          	addi	a5,a5,-2048 # 800 <printf_core+0x1c8>
     bac:	00f4f7b3          	and	a5,s1,a5
     bb0:	08079a63          	bnez	a5,c44 <printf_core+0x60c>
            } else if (fl & PAD_POS) {
     bb4:	0014f793          	andi	a5,s1,1
     bb8:	08078e63          	beqz	a5,c54 <printf_core+0x61c>
            pl = 1;
     bbc:	00100413          	li	s0,1
              prefix += 2;
     bc0:	00000c97          	auipc	s9,0x0
     bc4:	54ac8c93          	addi	s9,s9,1354 # 110a <__iob+0x62>
     bc8:	0240006f          	j	bec <printf_core+0x5b4>
              arg.i = -arg.i;
     bcc:	40f007b3          	neg	a5,a5
     bd0:	04f13023          	sd	a5,64(sp)
            pl = 1;
     bd4:	00100413          	li	s0,1
    prefix = "-+   0X0x";
     bd8:	00000c97          	auipc	s9,0x0
     bdc:	530c8c93          	addi	s9,s9,1328 # 1108 <__iob+0x60>
     be0:	00c0006f          	j	bec <printf_core+0x5b4>
    switch (t) {
     be4:	00000c97          	auipc	s9,0x0
     be8:	524c8c93          	addi	s9,s9,1316 # 1108 <__iob+0x60>
            a = fmt_u(arg.i, z);
     bec:	04010593          	addi	a1,sp,64
     bf0:	04013503          	ld	a0,64(sp)
     bf4:	8e9ff0ef          	jal	4dc <fmt_u>
     bf8:	00050a13          	mv	s4,a0
        if (xp && p < 0)
     bfc:	00813783          	ld	a5,8(sp)
     c00:	00078a63          	beqz	a5,c14 <printf_core+0x5dc>
     c04:	2e0d4063          	bltz	s10,ee4 <printf_core+0x8ac>
          fl &= ~ZERO_PAD;
     c08:	ffff07b7          	lui	a5,0xffff0
     c0c:	fff78793          	addi	a5,a5,-1 # fffffffffffeffff <seed+0xfffffffffffee9b7>
     c10:	00f4f4b3          	and	s1,s1,a5
        if (!arg.i && !p) {
     c14:	04013703          	ld	a4,64(sp)
     c18:	00071463          	bnez	a4,c20 <printf_core+0x5e8>
     c1c:	1c0d0863          	beqz	s10,dec <printf_core+0x7b4>
        p = MAX(p, z - a + !arg.i);
     c20:	04010793          	addi	a5,sp,64
     c24:	414787b3          	sub	a5,a5,s4
     c28:	00173713          	seqz	a4,a4
     c2c:	00e787b3          	add	a5,a5,a4
     c30:	00fd5463          	bge	s10,a5,c38 <printf_core+0x600>
     c34:	00078d13          	mv	s10,a5
     c38:	000d0d1b          	sext.w	s10,s10
    z = buf + sizeof(buf);
     c3c:	04010913          	addi	s2,sp,64
        break;
     c40:	0f00006f          	j	d30 <printf_core+0x6f8>
            pl = 1;
     c44:	00100413          	li	s0,1
              prefix++;
     c48:	00000c97          	auipc	s9,0x0
     c4c:	4c1c8c93          	addi	s9,s9,1217 # 1109 <__iob+0x61>
     c50:	f9dff06f          	j	bec <printf_core+0x5b4>
    prefix = "-+   0X0x";
     c54:	00000c97          	auipc	s9,0x0
     c58:	4b4c8c93          	addi	s9,s9,1204 # 1108 <__iob+0x60>
     c5c:	f91ff06f          	j	bec <printf_core+0x5b4>
     c60:	00000c97          	auipc	s9,0x0
     c64:	4a8c8c93          	addi	s9,s9,1192 # 1108 <__iob+0x60>
     c68:	f95ff06f          	j	bfc <printf_core+0x5c4>
     c6c:	00000c97          	auipc	s9,0x0
     c70:	49cc8c93          	addi	s9,s9,1180 # 1108 <__iob+0x60>
     c74:	f89ff06f          	j	bfc <printf_core+0x5c4>
     c78:	00000c97          	auipc	s9,0x0
     c7c:	490c8c93          	addi	s9,s9,1168 # 1108 <__iob+0x60>
     c80:	f7dff06f          	j	bfc <printf_core+0x5c4>
     c84:	00000c97          	auipc	s9,0x0
     c88:	484c8c93          	addi	s9,s9,1156 # 1108 <__iob+0x60>
     c8c:	f71ff06f          	j	bfc <printf_core+0x5c4>
        *(a = z - (p = 1)) = arg.i;
     c90:	04013783          	ld	a5,64(sp)
     c94:	02f10fa3          	sb	a5,63(sp)
        fl &= ~ZERO_PAD;
     c98:	ffff07b7          	lui	a5,0xffff0
     c9c:	fff78793          	addi	a5,a5,-1 # fffffffffffeffff <seed+0xfffffffffffee9b7>
     ca0:	00f4f4b3          	and	s1,s1,a5
    prefix = "-+   0X0x";
     ca4:	00000c97          	auipc	s9,0x0
     ca8:	464c8c93          	addi	s9,s9,1124 # 1108 <__iob+0x60>
        *(a = z - (p = 1)) = arg.i;
     cac:	00100d13          	li	s10,1
    z = buf + sizeof(buf);
     cb0:	04010913          	addi	s2,sp,64
        *(a = z - (p = 1)) = arg.i;
     cb4:	03f10a13          	addi	s4,sp,63
        break;
     cb8:	0780006f          	j	d30 <printf_core+0x6f8>
        a = arg.p ? arg.p : "(null)";
     cbc:	04013a03          	ld	s4,64(sp)
     cc0:	020a0e63          	beqz	s4,cfc <printf_core+0x6c4>
        z = a + strnlen(a, p < 0 ? INT_MAX : p);
     cc4:	040d4263          	bltz	s10,d08 <printf_core+0x6d0>
     cc8:	000d0593          	mv	a1,s10
     ccc:	000a0513          	mv	a0,s4
     cd0:	db8ff0ef          	jal	288 <strnlen>
     cd4:	00050793          	mv	a5,a0
     cd8:	00aa0933          	add	s2,s4,a0
        if (p < 0 && *z)
     cdc:	020d4c63          	bltz	s10,d14 <printf_core+0x6dc>
        p = z - a;
     ce0:	00078d1b          	sext.w	s10,a5
        fl &= ~ZERO_PAD;
     ce4:	ffff07b7          	lui	a5,0xffff0
     ce8:	fff78793          	addi	a5,a5,-1 # fffffffffffeffff <seed+0xfffffffffffee9b7>
     cec:	00f4f4b3          	and	s1,s1,a5
    prefix = "-+   0X0x";
     cf0:	00000c97          	auipc	s9,0x0
     cf4:	418c8c93          	addi	s9,s9,1048 # 1108 <__iob+0x60>
        break;
     cf8:	0380006f          	j	d30 <printf_core+0x6f8>
        a = arg.p ? arg.p : "(null)";
     cfc:	00000a17          	auipc	s4,0x0
     d00:	41ca0a13          	addi	s4,s4,1052 # 1118 <__iob+0x70>
     d04:	fc1ff06f          	j	cc4 <printf_core+0x68c>
        z = a + strnlen(a, p < 0 ? INT_MAX : p);
     d08:	800005b7          	lui	a1,0x80000
     d0c:	fff5c593          	not	a1,a1
     d10:	fbdff06f          	j	ccc <printf_core+0x694>
        if (p < 0 && *z)
     d14:	00094703          	lbu	a4,0(s2)
     d18:	fc0704e3          	beqz	a4,ce0 <printf_core+0x6a8>
inval:
  // errno = EINVAL;
  // return -1;
overflow:
  // errno = EOVERFLOW;
  return -1;
     d1c:	fff00a93          	li	s5,-1
     d20:	16c0006f          	j	e8c <printf_core+0x854>
    switch (t) {
     d24:	00000c97          	auipc	s9,0x0
     d28:	3e4c8c93          	addi	s9,s9,996 # 1108 <__iob+0x60>
     d2c:	04010913          	addi	s2,sp,64
    if (p < z - a)
     d30:	41490933          	sub	s2,s2,s4
     d34:	012d5463          	bge	s10,s2,d3c <printf_core+0x704>
      p = z - a;
     d38:	00090d1b          	sext.w	s10,s2
    if (p > INT_MAX - pl)
     d3c:	800007b7          	lui	a5,0x80000
     d40:	fff7879b          	addiw	a5,a5,-1 # 7fffffff <seed+0x7fffe9b7>
     d44:	408787bb          	subw	a5,a5,s0
     d48:	1ba7c263          	blt	a5,s10,eec <printf_core+0x8b4>
    if (w < pl + p)
     d4c:	008d0bbb          	addw	s7,s10,s0
     d50:	017c5463          	bge	s8,s7,d58 <printf_core+0x720>
      w = pl + p;
     d54:	000b8c13          	mv	s8,s7
    if (w > INT_MAX - cnt)
     d58:	1989ce63          	blt	s3,s8,ef4 <printf_core+0x8bc>
    pad(f, ' ', w, pl + p, fl);
     d5c:	00048713          	mv	a4,s1
     d60:	000b8693          	mv	a3,s7
     d64:	000c0613          	mv	a2,s8
     d68:	02000593          	li	a1,32
     d6c:	000b0513          	mv	a0,s6
     d70:	83dff0ef          	jal	5ac <pad>
    out(f, prefix, pl);
     d74:	00040613          	mv	a2,s0
     d78:	000c8593          	mv	a1,s9
     d7c:	000b0513          	mv	a0,s6
     d80:	ee0ff0ef          	jal	460 <out>
    pad(f, '0', w, pl + p, fl ^ ZERO_PAD);
     d84:	00010737          	lui	a4,0x10
     d88:	00e4c733          	xor	a4,s1,a4
     d8c:	000b8693          	mv	a3,s7
     d90:	000c0613          	mv	a2,s8
     d94:	03000593          	li	a1,48
     d98:	000b0513          	mv	a0,s6
     d9c:	811ff0ef          	jal	5ac <pad>
    pad(f, '0', p, z - a, 0);
     da0:	00000713          	li	a4,0
     da4:	00090693          	mv	a3,s2
     da8:	000d0613          	mv	a2,s10
     dac:	03000593          	li	a1,48
     db0:	000b0513          	mv	a0,s6
     db4:	ff8ff0ef          	jal	5ac <pad>
    out(f, a, z - a);
     db8:	00090613          	mv	a2,s2
     dbc:	000a0593          	mv	a1,s4
     dc0:	000b0513          	mv	a0,s6
     dc4:	e9cff0ef          	jal	460 <out>
    pad(f, ' ', w, pl + p, fl ^ LEFT_ADJ);
     dc8:	00002737          	lui	a4,0x2
     dcc:	00e4c733          	xor	a4,s1,a4
     dd0:	000b8693          	mv	a3,s7
     dd4:	000c0613          	mv	a2,s8
     dd8:	02000593          	li	a1,32
     ddc:	000b0513          	mv	a0,s6
     de0:	fccff0ef          	jal	5ac <pad>
    l = w;
     de4:	000c0413          	mv	s0,s8
     de8:	921ff06f          	j	708 <printf_core+0xd0>
    z = buf + sizeof(buf);
     dec:	04010913          	addi	s2,sp,64
          a = z;
     df0:	00090a13          	mv	s4,s2
     df4:	f3dff06f          	j	d30 <printf_core+0x6f8>
    pop_arg(nl_arg + i, nl_type[i], ap);
     df8:	00341513          	slli	a0,s0,0x3
     dfc:	000d8613          	mv	a2,s11
     e00:	01013783          	ld	a5,16(sp)
     e04:	00a78533          	add	a0,a5,a0
     e08:	cacff0ef          	jal	2b4 <pop_arg>
  for (i = 1; i <= NL_ARGMAX && nl_type[i]; i++)
     e0c:	00140413          	addi	s0,s0,1
     e10:	00900793          	li	a5,9
     e14:	0287e063          	bltu	a5,s0,e34 <printf_core+0x7fc>
     e18:	00241793          	slli	a5,s0,0x2
     e1c:	01813703          	ld	a4,24(sp)
     e20:	00f707b3          	add	a5,a4,a5
     e24:	0007a583          	lw	a1,0(a5)
     e28:	fc0598e3          	bnez	a1,df8 <printf_core+0x7c0>
     e2c:	0080006f          	j	e34 <printf_core+0x7fc>
  for (; i <= NL_ARGMAX && !nl_type[i]; i++)
     e30:	00140413          	addi	s0,s0,1
     e34:	00900793          	li	a5,9
     e38:	0087ec63          	bltu	a5,s0,e50 <printf_core+0x818>
     e3c:	00241793          	slli	a5,s0,0x2
     e40:	01813703          	ld	a4,24(sp)
     e44:	00f707b3          	add	a5,a4,a5
     e48:	0007a783          	lw	a5,0(a5)
     e4c:	fe0782e3          	beqz	a5,e30 <printf_core+0x7f8>
  if (i <= NL_ARGMAX)
     e50:	00900793          	li	a5,9
     e54:	0a87f863          	bgeu	a5,s0,f04 <printf_core+0x8cc>
  return 1;
     e58:	00100a93          	li	s5,1
     e5c:	0300006f          	j	e8c <printf_core+0x854>
  return -1;
     e60:	fff00a93          	li	s5,-1
     e64:	0280006f          	j	e8c <printf_core+0x854>
     e68:	fff00a93          	li	s5,-1
     e6c:	0200006f          	j	e8c <printf_core+0x854>
     e70:	fff00a93          	li	s5,-1
     e74:	0180006f          	j	e8c <printf_core+0x854>
     e78:	fff00a93          	li	s5,-1
     e7c:	0100006f          	j	e8c <printf_core+0x854>
     e80:	fff00a93          	li	s5,-1
     e84:	0080006f          	j	e8c <printf_core+0x854>
     e88:	fff00a93          	li	s5,-1
}
     e8c:	000a8513          	mv	a0,s5
     e90:	0b813083          	ld	ra,184(sp)
     e94:	0b013403          	ld	s0,176(sp)
     e98:	0a813483          	ld	s1,168(sp)
     e9c:	0a013903          	ld	s2,160(sp)
     ea0:	09813983          	ld	s3,152(sp)
     ea4:	09013a03          	ld	s4,144(sp)
     ea8:	08813a83          	ld	s5,136(sp)
     eac:	08013b03          	ld	s6,128(sp)
     eb0:	07813b83          	ld	s7,120(sp)
     eb4:	07013c03          	ld	s8,112(sp)
     eb8:	06813c83          	ld	s9,104(sp)
     ebc:	06013d03          	ld	s10,96(sp)
     ec0:	05813d83          	ld	s11,88(sp)
     ec4:	0c010113          	addi	sp,sp,192
     ec8:	00008067          	ret
  return -1;
     ecc:	fff00a93          	li	s5,-1
     ed0:	fbdff06f          	j	e8c <printf_core+0x854>
     ed4:	fff00a93          	li	s5,-1
     ed8:	fb5ff06f          	j	e8c <printf_core+0x854>
        return 0;
     edc:	00040a93          	mv	s5,s0
     ee0:	fadff06f          	j	e8c <printf_core+0x854>
  return -1;
     ee4:	fff00a93          	li	s5,-1
     ee8:	fa5ff06f          	j	e8c <printf_core+0x854>
     eec:	fff00a93          	li	s5,-1
     ef0:	f9dff06f          	j	e8c <printf_core+0x854>
     ef4:	fff00a93          	li	s5,-1
     ef8:	f95ff06f          	j	e8c <printf_core+0x854>
    return 0;
     efc:	00000a93          	li	s5,0
     f00:	f8dff06f          	j	e8c <printf_core+0x854>
  return -1;
     f04:	fff00a93          	li	s5,-1
     f08:	f85ff06f          	j	e8c <printf_core+0x854>

0000000000000f0c <vfprintf>:
  return ret;
}

#else

int vfprintf(FILE *restrict f, const char *restrict fmt, va_list ap) {
     f0c:	f5010113          	addi	sp,sp,-176
     f10:	0a113423          	sd	ra,168(sp)
     f14:	0a813023          	sd	s0,160(sp)
     f18:	08913c23          	sd	s1,152(sp)
     f1c:	00050413          	mv	s0,a0
     f20:	00058493          	mv	s1,a1
     f24:	00c13423          	sd	a2,8(sp)
  int nl_type[NL_ARGMAX + 1] = {0};
     f28:	06013423          	sd	zero,104(sp)
     f2c:	06013823          	sd	zero,112(sp)
     f30:	06013c23          	sd	zero,120(sp)
     f34:	08013023          	sd	zero,128(sp)
     f38:	08013423          	sd	zero,136(sp)
  union arg nl_arg[NL_ARGMAX + 1];

  // preprocess nl arguments
  va_list ap2;
  va_copy(ap2, ap);
     f3c:	00c13823          	sd	a2,16(sp)
  int ret = printf_core(0, fmt, &ap2, nl_arg, nl_type);
     f40:	06810713          	addi	a4,sp,104
     f44:	01810693          	addi	a3,sp,24
     f48:	01010613          	addi	a2,sp,16
     f4c:	00000513          	li	a0,0
     f50:	ee8ff0ef          	jal	638 <printf_core>
  va_end(ap2);

  if (ret < 0) {
     f54:	00054e63          	bltz	a0,f70 <vfprintf+0x64>
    return ret;
  }
  return printf_core(f, fmt, &ap, nl_arg, nl_type);
     f58:	06810713          	addi	a4,sp,104
     f5c:	01810693          	addi	a3,sp,24
     f60:	00810613          	addi	a2,sp,8
     f64:	00048593          	mv	a1,s1
     f68:	00040513          	mv	a0,s0
     f6c:	eccff0ef          	jal	638 <printf_core>
}
     f70:	0a813083          	ld	ra,168(sp)
     f74:	0a013403          	ld	s0,160(sp)
     f78:	09813483          	ld	s1,152(sp)
     f7c:	0b010113          	addi	sp,sp,176
     f80:	00008067          	ret

0000000000000f84 <__udivsi3>:
# define __divdi3 __divsi3
# define __moddi3 __modsi3
#else
FUNC_BEGIN (__udivsi3)
  /* Compute __udivdi3(a0 << 32, a1 << 32); cast result to uint32_t.  */
  sll    a0, a0, 32
     f84:	02051513          	slli	a0,a0,0x20
  sll    a1, a1, 32
     f88:	02059593          	slli	a1,a1,0x20
  move   t0, ra
     f8c:	00008293          	mv	t0,ra
  jal    HIDDEN_JUMPTARGET(__udivdi3)
     f90:	03c000ef          	jal	fcc <__hidden___udivdi3>
  sext.w a0, a0
     f94:	0005051b          	sext.w	a0,a0
  jr     t0
     f98:	00028067          	jr	t0

0000000000000f9c <__umodsi3>:
FUNC_END (__udivsi3)

FUNC_BEGIN (__umodsi3)
  /* Compute __udivdi3((uint32_t)a0, (uint32_t)a1); cast a1 to uint32_t.  */
  sll    a0, a0, 32
     f9c:	02051513          	slli	a0,a0,0x20
  sll    a1, a1, 32
     fa0:	02059593          	slli	a1,a1,0x20
  srl    a0, a0, 32
     fa4:	02055513          	srli	a0,a0,0x20
  srl    a1, a1, 32
     fa8:	0205d593          	srli	a1,a1,0x20
  move   t0, ra
     fac:	00008293          	mv	t0,ra
  jal    HIDDEN_JUMPTARGET(__udivdi3)
     fb0:	01c000ef          	jal	fcc <__hidden___udivdi3>
  sext.w a0, a1
     fb4:	0005851b          	sext.w	a0,a1
  jr     t0
     fb8:	00028067          	jr	t0

0000000000000fbc <__divsi3>:

FUNC_ALIAS (__modsi3, __moddi3)

FUNC_BEGIN( __divsi3)
  /* Check for special case of INT_MIN/-1. Otherwise, fall into __divdi3.  */
  li    t0, -1
     fbc:	fff00293          	li	t0,-1
  beq   a1, t0, .L20
     fc0:	0a558c63          	beq	a1,t0,1078 <__moddi3+0x30>

0000000000000fc4 <__divdi3>:
#endif

FUNC_BEGIN (__divdi3)
  bltz  a0, .L10
     fc4:	06054063          	bltz	a0,1024 <__umoddi3+0x10>
  bltz  a1, .L11
     fc8:	0605c663          	bltz	a1,1034 <__umoddi3+0x20>

0000000000000fcc <__hidden___udivdi3>:
  /* Since the quotient is positive, fall into __udivdi3.  */

FUNC_BEGIN (__udivdi3)
  mv    a2, a1
     fcc:	00058613          	mv	a2,a1
  mv    a1, a0
     fd0:	00050593          	mv	a1,a0
  li    a0, -1
     fd4:	fff00513          	li	a0,-1
  beqz  a2, .L5
     fd8:	02060c63          	beqz	a2,1010 <__hidden___udivdi3+0x44>
  li    a3, 1
     fdc:	00100693          	li	a3,1
  bgeu  a2, a1, .L2
     fe0:	00b67a63          	bgeu	a2,a1,ff4 <__hidden___udivdi3+0x28>
.L1:
  blez  a2, .L2
     fe4:	00c05863          	blez	a2,ff4 <__hidden___udivdi3+0x28>
  slli  a2, a2, 1
     fe8:	00161613          	slli	a2,a2,0x1
  slli  a3, a3, 1
     fec:	00169693          	slli	a3,a3,0x1
  bgtu  a1, a2, .L1
     ff0:	feb66ae3          	bltu	a2,a1,fe4 <__hidden___udivdi3+0x18>
.L2:
  li    a0, 0
     ff4:	00000513          	li	a0,0
.L3:
  bltu  a1, a2, .L4
     ff8:	00c5e663          	bltu	a1,a2,1004 <__hidden___udivdi3+0x38>
  sub   a1, a1, a2
     ffc:	40c585b3          	sub	a1,a1,a2
  or    a0, a0, a3
    1000:	00d56533          	or	a0,a0,a3
.L4:
  srli  a3, a3, 1
    1004:	0016d693          	srli	a3,a3,0x1
  srli  a2, a2, 1
    1008:	00165613          	srli	a2,a2,0x1
  bnez  a3, .L3
    100c:	fe0696e3          	bnez	a3,ff8 <__hidden___udivdi3+0x2c>
.L5:
  ret
    1010:	00008067          	ret

0000000000001014 <__umoddi3>:
FUNC_END (__udivdi3)
HIDDEN_DEF (__udivdi3)

FUNC_BEGIN (__umoddi3)
  /* Call __udivdi3(a0, a1), then return the remainder, which is in a1.  */
  move  t0, ra
    1014:	00008293          	mv	t0,ra
  jal   HIDDEN_JUMPTARGET(__udivdi3)
    1018:	fb5ff0ef          	jal	fcc <__hidden___udivdi3>
  move  a0, a1
    101c:	00058513          	mv	a0,a1
  jr    t0
    1020:	00028067          	jr	t0
FUNC_END (__umoddi3)

  /* Handle negative arguments to __divdi3.  */
.L10:
  neg   a0, a0
    1024:	40a00533          	neg	a0,a0
  /* Zero is handled as a negative so that the result will not be inverted.  */
  bgtz  a1, .L12     /* Compute __udivdi3(-a0, a1), then negate the result.  */
    1028:	00b04863          	bgtz	a1,1038 <__umoddi3+0x24>

  neg   a1, a1
    102c:	40b005b3          	neg	a1,a1
  j     HIDDEN_JUMPTARGET(__udivdi3)     /* Compute __udivdi3(-a0, -a1).  */
    1030:	f9dff06f          	j	fcc <__hidden___udivdi3>
.L11:                /* Compute __udivdi3(a0, -a1), then negate the result.  */
  neg   a1, a1
    1034:	40b005b3          	neg	a1,a1
.L12:
  move  t0, ra
    1038:	00008293          	mv	t0,ra
  jal   HIDDEN_JUMPTARGET(__udivdi3)
    103c:	f91ff0ef          	jal	fcc <__hidden___udivdi3>
  neg   a0, a0
    1040:	40a00533          	neg	a0,a0
  jr    t0
    1044:	00028067          	jr	t0

0000000000001048 <__moddi3>:
FUNC_END (__divdi3)

FUNC_BEGIN (__moddi3)
  move   t0, ra
    1048:	00008293          	mv	t0,ra
  bltz   a1, .L31
    104c:	0005ca63          	bltz	a1,1060 <__moddi3+0x18>
  bltz   a0, .L32
    1050:	00054c63          	bltz	a0,1068 <__moddi3+0x20>
.L30:
  jal    HIDDEN_JUMPTARGET(__udivdi3)    /* The dividend is not negative.  */
    1054:	f79ff0ef          	jal	fcc <__hidden___udivdi3>
  move   a0, a1
    1058:	00058513          	mv	a0,a1
  jr     t0
    105c:	00028067          	jr	t0
.L31:
  neg    a1, a1
    1060:	40b005b3          	neg	a1,a1
  bgez   a0, .L30
    1064:	fe0558e3          	bgez	a0,1054 <__moddi3+0xc>
.L32:
  neg    a0, a0
    1068:	40a00533          	neg	a0,a0
  jal    HIDDEN_JUMPTARGET(__udivdi3)    /* The dividend is hella negative.  */
    106c:	f61ff0ef          	jal	fcc <__hidden___udivdi3>
  neg    a0, a1
    1070:	40b00533          	neg	a0,a1
  jr     t0
    1074:	00028067          	jr	t0
FUNC_END (__moddi3)

#if __riscv_xlen == 64
  /* continuation of __divsi3 */
.L20:
  sll   t0, t0, 31
    1078:	01f29293          	slli	t0,t0,0x1f
  bne   a0, t0, __divdi3
    107c:	f45514e3          	bne	a0,t0,fc4 <__divdi3>
  ret
    1080:	00008067          	ret

0000000000001084 <__muldi3>:
/* Our RV64 64-bit routine is equivalent to our RV32 32-bit routine.  */
# define __muldi3 __mulsi3
#endif

FUNC_BEGIN (__muldi3)
  mv     a2, a0
    1084:	00050613          	mv	a2,a0
  li     a0, 0
    1088:	00000513          	li	a0,0
.L1:
  andi   a3, a1, 1
    108c:	0015f693          	andi	a3,a1,1
  beqz   a3, .L2
    1090:	00068463          	beqz	a3,1098 <__muldi3+0x14>
  add    a0, a0, a2
    1094:	00c50533          	add	a0,a0,a2
.L2:
  srli   a1, a1, 1
    1098:	0015d593          	srli	a1,a1,0x1
  slli   a2, a2, 1
    109c:	00161613          	slli	a2,a2,0x1
  bnez   a1, .L1
    10a0:	fe0596e3          	bnez	a1,108c <__muldi3+0x8>
  ret
    10a4:	00008067          	ret
