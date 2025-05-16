
uapp.elf:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <_start>:
    .section .text.init
    .globl _start
_start:
    j main
       0:	0740006f          	j	74 <main>

0000000000000004 <user_clock>:
#define USER_MAIN PFH1
#endif

#define DELAY_TIME 1247

static uint64_t user_clock(void) {
       4:	ff010113          	addi	sp,sp,-16
       8:	00113423          	sd	ra,8(sp)
  uint64_t ret;
  asm volatile("rdtime %0" : "=r"(ret));
       c:	c0102573          	rdtime	a0
  return ret / 10;
      10:	00a00593          	li	a1,10
      14:	7a1000ef          	jal	fb4 <__hidden___udivdi3>
}
      18:	00813083          	ld	ra,8(sp)
      1c:	01010113          	addi	sp,sp,16
      20:	00008067          	ret

0000000000000024 <delay>:

static void delay(unsigned long ms) {
      24:	fe010113          	addi	sp,sp,-32
      28:	00113c23          	sd	ra,24(sp)
      2c:	00813823          	sd	s0,16(sp)
      30:	00913423          	sd	s1,8(sp)
      34:	00050413          	mv	s0,a0
  uint64_t prev_clock = user_clock();
      38:	fcdff0ef          	jal	4 <user_clock>
      3c:	00050493          	mv	s1,a0
  while (user_clock() - prev_clock < ms * 1000)
      40:	fc5ff0ef          	jal	4 <user_clock>
      44:	40950533          	sub	a0,a0,s1
      48:	00541793          	slli	a5,s0,0x5
      4c:	408787b3          	sub	a5,a5,s0
      50:	00279793          	slli	a5,a5,0x2
      54:	008787b3          	add	a5,a5,s0
      58:	00379793          	slli	a5,a5,0x3
      5c:	fef562e3          	bltu	a0,a5,40 <delay+0x1c>
    ;
}
      60:	01813083          	ld	ra,24(sp)
      64:	01013403          	ld	s0,16(sp)
      68:	00813483          	ld	s1,8(sp)
      6c:	02010113          	addi	sp,sp,32
      70:	00008067          	ret

0000000000000074 <main>:

#if USER_MAIN == PFH1

int main(void) {
      74:	ff010113          	addi	sp,sp,-16
      78:	00113423          	sd	ra,8(sp)
  register const void *const sp asm("sp");

  while (1) {
    printf("\x1b[44m[U]\x1b[0m [PID = %d, sp = %p]\n", getpid(), sp);
      7c:	0b4000ef          	jal	130 <getpid>
      80:	00050593          	mv	a1,a0
      84:	00010613          	mv	a2,sp
      88:	00001517          	auipc	a0,0x1
      8c:	00850513          	addi	a0,a0,8 # 1090 <__muldi3+0x24>
      90:	058000ef          	jal	e8 <printf>
    delay(DELAY_TIME);
      94:	4df00513          	li	a0,1247
      98:	f8dff0ef          	jal	24 <delay>
  while (1) {
      9c:	fe1ff06f          	j	7c <main+0x8>

00000000000000a0 <fileno>:
  return (int)write(fileno(fp), buf, len);
}

int fileno(FILE *restrict f) {
  return f->fd;
}
      a0:	00052503          	lw	a0,0(a0)
      a4:	00008067          	ret

00000000000000a8 <printf_syscall_write>:
static int printf_syscall_write(FILE *restrict fp, const void *restrict buf, size_t len) {
      a8:	fe010113          	addi	sp,sp,-32
      ac:	00113c23          	sd	ra,24(sp)
      b0:	00813823          	sd	s0,16(sp)
      b4:	00913423          	sd	s1,8(sp)
      b8:	00058413          	mv	s0,a1
      bc:	00060493          	mv	s1,a2
  return (int)write(fileno(fp), buf, len);
      c0:	fe1ff0ef          	jal	a0 <fileno>
      c4:	00048613          	mv	a2,s1
      c8:	00040593          	mv	a1,s0
      cc:	078000ef          	jal	144 <write>
}
      d0:	0005051b          	sext.w	a0,a0
      d4:	01813083          	ld	ra,24(sp)
      d8:	01013403          	ld	s0,16(sp)
      dc:	00813483          	ld	s1,8(sp)
      e0:	02010113          	addi	sp,sp,32
      e4:	00008067          	ret

00000000000000e8 <printf>:

int printf(const char *restrict fmt, ...) {
      e8:	fa010113          	addi	sp,sp,-96
      ec:	00113c23          	sd	ra,24(sp)
      f0:	02b13423          	sd	a1,40(sp)
      f4:	02c13823          	sd	a2,48(sp)
      f8:	02d13c23          	sd	a3,56(sp)
      fc:	04e13023          	sd	a4,64(sp)
     100:	04f13423          	sd	a5,72(sp)
     104:	05013823          	sd	a6,80(sp)
     108:	05113c23          	sd	a7,88(sp)
  va_list ap;
  va_start(ap, fmt);
     10c:	02810613          	addi	a2,sp,40
     110:	00c13423          	sd	a2,8(sp)
  int ret = vfprintf(stdout, fmt, ap);
     114:	00050593          	mv	a1,a0
     118:	00001517          	auipc	a0,0x1
     11c:	50050513          	addi	a0,a0,1280 # 1618 <__iob+0x10>
     120:	5d5000ef          	jal	ef4 <vfprintf>
  va_end(ap);
  return ret;
}
     124:	01813083          	ld	ra,24(sp)
     128:	06010113          	addi	sp,sp,96
     12c:	00008067          	ret

0000000000000130 <getpid>:
#include <stdint.h>
#include <time.h>

pid_t getpid(void) {
  pid_t ret;
  asm volatile("li a7, %1\n\t"
     130:	0ac00893          	li	a7,172
     134:	00000073          	ecall
     138:	00050793          	mv	a5,a0
               "mv %0, a0\n\t"
               : "=r"(ret)
               : "i"(__NR_getpid)
               : "a0", "a7", "memory");
  return ret;
}
     13c:	0007851b          	sext.w	a0,a5
     140:	00008067          	ret

0000000000000144 <write>:

ssize_t write(int fd, const void *buf, size_t count) {
     144:	00050793          	mv	a5,a0
     148:	00058713          	mv	a4,a1
     14c:	00060693          	mv	a3,a2
  ssize_t ret;
  asm volatile("li a7, %1\n\t"
     150:	04000893          	li	a7,64
     154:	00078513          	mv	a0,a5
     158:	00070593          	mv	a1,a4
     15c:	00068613          	mv	a2,a3
     160:	00000073          	ecall
     164:	00050793          	mv	a5,a0
               : "=r" (ret)
               : "i"(__NR_write), "r" (fd), "r" (buf), "r" (count)
               : "a0", "a1", "a2", "a7", "memory"
  );
  return ret;
}
     168:	00078513          	mv	a0,a5
     16c:	00008067          	ret

0000000000000170 <srand>:
#include <stdint.h>

static uint64_t seed;

void srand(unsigned s) {
  seed = s - 1;
     170:	fff5051b          	addiw	a0,a0,-1
     174:	02051513          	slli	a0,a0,0x20
     178:	02055513          	srli	a0,a0,0x20
     17c:	00001797          	auipc	a5,0x1
     180:	4aa7be23          	sd	a0,1212(a5) # 1638 <seed>
}
     184:	00008067          	ret

0000000000000188 <rand>:

int rand(void) {
  seed = 6364136223846793005ULL * seed + 1;
     188:	00001617          	auipc	a2,0x1
     18c:	4b060613          	addi	a2,a2,1200 # 1638 <seed>
     190:	00063783          	ld	a5,0(a2)
     194:	00479693          	slli	a3,a5,0x4
     198:	40f686b3          	sub	a3,a3,a5
     19c:	00669713          	slli	a4,a3,0x6
     1a0:	40d70733          	sub	a4,a4,a3
     1a4:	00771693          	slli	a3,a4,0x7
     1a8:	00d70733          	add	a4,a4,a3
     1ac:	00271693          	slli	a3,a4,0x2
     1b0:	00f68733          	add	a4,a3,a5
     1b4:	00671693          	slli	a3,a4,0x6
     1b8:	40e68733          	sub	a4,a3,a4
     1bc:	00771693          	slli	a3,a4,0x7
     1c0:	00f686b3          	add	a3,a3,a5
     1c4:	00269713          	slli	a4,a3,0x2
     1c8:	00f70733          	add	a4,a4,a5
     1cc:	00371693          	slli	a3,a4,0x3
     1d0:	40e686b3          	sub	a3,a3,a4
     1d4:	00369713          	slli	a4,a3,0x3
     1d8:	40d70733          	sub	a4,a4,a3
     1dc:	00671693          	slli	a3,a4,0x6
     1e0:	40e686b3          	sub	a3,a3,a4
     1e4:	00269713          	slli	a4,a3,0x2
     1e8:	40f70733          	sub	a4,a4,a5
     1ec:	00771693          	slli	a3,a4,0x7
     1f0:	40f686b3          	sub	a3,a3,a5
     1f4:	00269713          	slli	a4,a3,0x2
     1f8:	00f70733          	add	a4,a4,a5
     1fc:	00271693          	slli	a3,a4,0x2
     200:	40f686b3          	sub	a3,a3,a5
     204:	00269713          	slli	a4,a3,0x2
     208:	40f70733          	sub	a4,a4,a5
     20c:	00271513          	slli	a0,a4,0x2
     210:	00f50533          	add	a0,a0,a5
     214:	00150513          	addi	a0,a0,1
     218:	00a63023          	sd	a0,0(a2)
  return seed >> 33;
}
     21c:	02155513          	srli	a0,a0,0x21
     220:	00008067          	ret

0000000000000224 <memset>:
#include <string.h>

void *memset(void *restrict dst, int c, size_t n) {
    unsigned char *p = dst;
    unsigned char val = (unsigned char)c;
     224:	0ff5f593          	zext.b	a1,a1
    unsigned char *p = dst;
     228:	00050713          	mv	a4,a0
    for (size_t i = 0; i < n; i++) {
     22c:	00000793          	li	a5,0
     230:	0100006f          	j	240 <memset+0x1c>
        *p++ = val;
     234:	00b70023          	sb	a1,0(a4)
    for (size_t i = 0; i < n; i++) {
     238:	00178793          	addi	a5,a5,1
        *p++ = val;
     23c:	00170713          	addi	a4,a4,1
    for (size_t i = 0; i < n; i++) {
     240:	fec7eae3          	bltu	a5,a2,234 <memset+0x10>
    }
    return dst;
}
     244:	00008067          	ret

0000000000000248 <memcpy>:

void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
    unsigned char *p_dst = dst;
     248:	00050713          	mv	a4,a0
    const unsigned char *p_src = src;
    for (size_t i = 0; i < n; i++) {
     24c:	00000793          	li	a5,0
     250:	0180006f          	j	268 <memcpy+0x20>
        *p_dst++ = *p_src++;
     254:	0005c683          	lbu	a3,0(a1)
     258:	00d70023          	sb	a3,0(a4)
    for (size_t i = 0; i < n; i++) {
     25c:	00178793          	addi	a5,a5,1
        *p_dst++ = *p_src++;
     260:	00158593          	addi	a1,a1,1
     264:	00170713          	addi	a4,a4,1
    for (size_t i = 0; i < n; i++) {
     268:	fec7e6e3          	bltu	a5,a2,254 <memcpy+0xc>
    }
    return dst;
}
     26c:	00008067          	ret

0000000000000270 <strnlen>:

size_t strnlen(const char *restrict s, size_t maxlen) {
     270:	00050713          	mv	a4,a0
    size_t len = 0;
     274:	00000513          	li	a0,0
    while (len < maxlen && s[len] != '\0') {
     278:	0080006f          	j	280 <strnlen+0x10>
        len++;
     27c:	00150513          	addi	a0,a0,1
    while (len < maxlen && s[len] != '\0') {
     280:	00b57863          	bgeu	a0,a1,290 <strnlen+0x20>
     284:	00a707b3          	add	a5,a4,a0
     288:	0007c783          	lbu	a5,0(a5)
     28c:	fe0798e3          	bnez	a5,27c <strnlen+0xc>
    }
    return len;
}
     290:	00008067          	ret

0000000000000294 <clock>:
#include "time.h"

clock_t clock(void) {
    clock_t ret;
    asm volatile("rdtime %0" : "=r"(ret));
     294:	c0102573          	rdtime	a0
    // return ret / (TIMECLOCK / CLOCKS_PER_SEC); // bug
    return ret;
}
     298:	00008067          	ret

000000000000029c <pop_arg>:
  // long double f;
  void *p;
};

static void pop_arg(union arg *arg, int type, va_list *ap) {
  switch (type) {
     29c:	ff85859b          	addiw	a1,a1,-8
     2a0:	00f00793          	li	a5,15
     2a4:	1ab7e063          	bltu	a5,a1,444 <pop_arg+0x1a8>
     2a8:	02059793          	slli	a5,a1,0x20
     2ac:	01e7d593          	srli	a1,a5,0x1e
     2b0:	00001717          	auipc	a4,0x1
     2b4:	e2070713          	addi	a4,a4,-480 # 10d0 <__muldi3+0x64>
     2b8:	00e585b3          	add	a1,a1,a4
     2bc:	0005a783          	lw	a5,0(a1)
     2c0:	00e787b3          	add	a5,a5,a4
     2c4:	00078067          	jr	a5
    case PTR:
      arg->p = va_arg(*ap, void *);
     2c8:	00063783          	ld	a5,0(a2)
     2cc:	00878713          	addi	a4,a5,8
     2d0:	00e63023          	sd	a4,0(a2)
     2d4:	0007b783          	ld	a5,0(a5)
     2d8:	00f53023          	sd	a5,0(a0)
      break;
     2dc:	00008067          	ret
    case INT:
      arg->i = va_arg(*ap, int);
     2e0:	00063783          	ld	a5,0(a2)
     2e4:	00878713          	addi	a4,a5,8
     2e8:	00e63023          	sd	a4,0(a2)
     2ec:	0007a783          	lw	a5,0(a5)
     2f0:	00f53023          	sd	a5,0(a0)
      break;
     2f4:	00008067          	ret
    case UINT:
      arg->i = va_arg(*ap, unsigned int);
     2f8:	00063783          	ld	a5,0(a2)
     2fc:	00878713          	addi	a4,a5,8
     300:	00e63023          	sd	a4,0(a2)
     304:	0007e783          	lwu	a5,0(a5)
     308:	00f53023          	sd	a5,0(a0)
      break;
     30c:	00008067          	ret
    case LONG:
      arg->i = va_arg(*ap, long);
     310:	00063783          	ld	a5,0(a2)
     314:	00878713          	addi	a4,a5,8
     318:	00e63023          	sd	a4,0(a2)
     31c:	0007b783          	ld	a5,0(a5)
     320:	00f53023          	sd	a5,0(a0)
      break;
     324:	00008067          	ret
    case ULONG:
      arg->i = va_arg(*ap, unsigned long);
     328:	00063783          	ld	a5,0(a2)
     32c:	00878713          	addi	a4,a5,8
     330:	00e63023          	sd	a4,0(a2)
     334:	0007b783          	ld	a5,0(a5)
     338:	00f53023          	sd	a5,0(a0)
      break;
     33c:	00008067          	ret
    case ULLONG:
      arg->i = va_arg(*ap, unsigned long long);
     340:	00063783          	ld	a5,0(a2)
     344:	00878713          	addi	a4,a5,8
     348:	00e63023          	sd	a4,0(a2)
     34c:	0007b783          	ld	a5,0(a5)
     350:	00f53023          	sd	a5,0(a0)
      break;
     354:	00008067          	ret
    case SHORT:
      arg->i = (short)va_arg(*ap, int);
     358:	00063783          	ld	a5,0(a2)
     35c:	00878713          	addi	a4,a5,8
     360:	00e63023          	sd	a4,0(a2)
     364:	00079783          	lh	a5,0(a5)
     368:	00f53023          	sd	a5,0(a0)
      break;
     36c:	00008067          	ret
    case USHORT:
      arg->i = (unsigned short)va_arg(*ap, int);
     370:	00063783          	ld	a5,0(a2)
     374:	00878713          	addi	a4,a5,8
     378:	00e63023          	sd	a4,0(a2)
     37c:	0007d783          	lhu	a5,0(a5)
     380:	00f53023          	sd	a5,0(a0)
      break;
     384:	00008067          	ret
    case CHAR:
      arg->i = (signed char)va_arg(*ap, int);
     388:	00063783          	ld	a5,0(a2)
     38c:	00878713          	addi	a4,a5,8
     390:	00e63023          	sd	a4,0(a2)
     394:	00078783          	lb	a5,0(a5)
     398:	00f53023          	sd	a5,0(a0)
      break;
     39c:	00008067          	ret
    case UCHAR:
      arg->i = (unsigned char)va_arg(*ap, int);
     3a0:	00063783          	ld	a5,0(a2)
     3a4:	00878713          	addi	a4,a5,8
     3a8:	00e63023          	sd	a4,0(a2)
     3ac:	0007c783          	lbu	a5,0(a5)
     3b0:	00f53023          	sd	a5,0(a0)
      break;
     3b4:	00008067          	ret
    case LLONG:
      arg->i = va_arg(*ap, long long);
     3b8:	00063783          	ld	a5,0(a2)
     3bc:	00878713          	addi	a4,a5,8
     3c0:	00e63023          	sd	a4,0(a2)
     3c4:	0007b783          	ld	a5,0(a5)
     3c8:	00f53023          	sd	a5,0(a0)
      break;
     3cc:	00008067          	ret
    case SIZET:
      arg->i = va_arg(*ap, size_t);
     3d0:	00063783          	ld	a5,0(a2)
     3d4:	00878713          	addi	a4,a5,8
     3d8:	00e63023          	sd	a4,0(a2)
     3dc:	0007b783          	ld	a5,0(a5)
     3e0:	00f53023          	sd	a5,0(a0)
      break;
     3e4:	00008067          	ret
    case IMAX:
      arg->i = va_arg(*ap, intmax_t);
     3e8:	00063783          	ld	a5,0(a2)
     3ec:	00878713          	addi	a4,a5,8
     3f0:	00e63023          	sd	a4,0(a2)
     3f4:	0007b783          	ld	a5,0(a5)
     3f8:	00f53023          	sd	a5,0(a0)
      break;
     3fc:	00008067          	ret
    case UMAX:
      arg->i = va_arg(*ap, uintmax_t);
     400:	00063783          	ld	a5,0(a2)
     404:	00878713          	addi	a4,a5,8
     408:	00e63023          	sd	a4,0(a2)
     40c:	0007b783          	ld	a5,0(a5)
     410:	00f53023          	sd	a5,0(a0)
      break;
     414:	00008067          	ret
    case PDIFF:
      arg->i = va_arg(*ap, ptrdiff_t);
     418:	00063783          	ld	a5,0(a2)
     41c:	00878713          	addi	a4,a5,8
     420:	00e63023          	sd	a4,0(a2)
     424:	0007b783          	ld	a5,0(a5)
     428:	00f53023          	sd	a5,0(a0)
      break;
     42c:	00008067          	ret
    case UIPTR:
      arg->i = (uintptr_t)va_arg(*ap, void *);
     430:	00063783          	ld	a5,0(a2)
     434:	00878713          	addi	a4,a5,8
     438:	00e63023          	sd	a4,0(a2)
     43c:	0007b783          	ld	a5,0(a5)
     440:	00f53023          	sd	a5,0(a0)
      //   arg->f = va_arg(*ap, double);
      //   break;
      // case LDBL:
      //   arg->f = va_arg(*ap, long double);
  }
}
     444:	00008067          	ret

0000000000000448 <out>:

static void out(FILE *f, const char *s, size_t l) {
     448:	ff010113          	addi	sp,sp,-16
     44c:	00113423          	sd	ra,8(sp)
  f->write(f, s, l);
     450:	00853783          	ld	a5,8(a0)
     454:	000780e7          	jalr	a5
}
     458:	00813083          	ld	ra,8(sp)
     45c:	01010113          	addi	sp,sp,16
     460:	00008067          	ret

0000000000000464 <fmt_x>:
  out(f, pad, l);
}

static const char xdigits[16] = {"0123456789ABCDEF"};

static char *fmt_x(uintmax_t x, char *s, int lower) {
     464:	00050793          	mv	a5,a0
     468:	00058513          	mv	a0,a1
  for (; x; x >>= 4)
     46c:	0280006f          	j	494 <fmt_x+0x30>
    *--s = xdigits[(x & 15)] | lower;
     470:	00f7f693          	andi	a3,a5,15
     474:	00001717          	auipc	a4,0x1
     478:	d3c70713          	addi	a4,a4,-708 # 11b0 <xdigits>
     47c:	00d70733          	add	a4,a4,a3
     480:	00074703          	lbu	a4,0(a4)
     484:	fff50513          	addi	a0,a0,-1
     488:	00c76733          	or	a4,a4,a2
     48c:	00e50023          	sb	a4,0(a0)
  for (; x; x >>= 4)
     490:	0047d793          	srli	a5,a5,0x4
     494:	fc079ee3          	bnez	a5,470 <fmt_x+0xc>
  return s;
}
     498:	00008067          	ret

000000000000049c <fmt_o>:

static char *fmt_o(uintmax_t x, char *s) {
     49c:	00050793          	mv	a5,a0
     4a0:	00058513          	mv	a0,a1
  for (; x; x >>= 3)
     4a4:	0180006f          	j	4bc <fmt_o+0x20>
    *--s = '0' + (x & 7);
     4a8:	0077f713          	andi	a4,a5,7
     4ac:	fff50513          	addi	a0,a0,-1
     4b0:	03070713          	addi	a4,a4,48
     4b4:	00e50023          	sb	a4,0(a0)
  for (; x; x >>= 3)
     4b8:	0037d793          	srli	a5,a5,0x3
     4bc:	fe0796e3          	bnez	a5,4a8 <fmt_o+0xc>
  return s;
}
     4c0:	00008067          	ret

00000000000004c4 <fmt_u>:

static char *fmt_u(uintmax_t x, char *s) {
     4c4:	fe010113          	addi	sp,sp,-32
     4c8:	00113c23          	sd	ra,24(sp)
     4cc:	00813823          	sd	s0,16(sp)
     4d0:	00913423          	sd	s1,8(sp)
     4d4:	00050413          	mv	s0,a0
     4d8:	00058493          	mv	s1,a1
  unsigned long y;
  for (; x > ULONG_MAX; x /= 10)
    *--s = '0' + x % 10;
  for (y = x; y; y /= 10)
     4dc:	02c0006f          	j	508 <fmt_u+0x44>
    *--s = '0' + y % 10;
     4e0:	00a00593          	li	a1,10
     4e4:	00040513          	mv	a0,s0
     4e8:	315000ef          	jal	ffc <__umoddi3>
     4ec:	fff48493          	addi	s1,s1,-1
     4f0:	0305051b          	addiw	a0,a0,48
     4f4:	00a48023          	sb	a0,0(s1)
  for (y = x; y; y /= 10)
     4f8:	00a00593          	li	a1,10
     4fc:	00040513          	mv	a0,s0
     500:	2b5000ef          	jal	fb4 <__hidden___udivdi3>
     504:	00050413          	mv	s0,a0
     508:	fc041ce3          	bnez	s0,4e0 <fmt_u+0x1c>
  return s;
}
     50c:	00048513          	mv	a0,s1
     510:	01813083          	ld	ra,24(sp)
     514:	01013403          	ld	s0,16(sp)
     518:	00813483          	ld	s1,8(sp)
     51c:	02010113          	addi	sp,sp,32
     520:	00008067          	ret

0000000000000524 <getint>:

static int getint(char **s) {
     524:	00050813          	mv	a6,a0
  int i;
  for (i = 0; isdigit(**s); (*s)++) {
     528:	00000513          	li	a0,0
     52c:	0100006f          	j	53c <getint+0x18>
    if (i > INT_MAX / 10 || **s - '0' > INT_MAX - 10 * i)
      i = -1;
     530:	fff00513          	li	a0,-1
  for (i = 0; isdigit(**s); (*s)++) {
     534:	00178793          	addi	a5,a5,1
     538:	00f83023          	sd	a5,0(a6)
     53c:	00083783          	ld	a5,0(a6)
     540:	0007c683          	lbu	a3,0(a5)
static inline int iscntrl(int c) {
  return (c >= 0 && c <= 0x1f) || c == 0x7f;
}

static inline int isdigit(int c) {
  return c >= '0' && c <= '9';
     544:	fd06861b          	addiw	a2,a3,-48
     548:	00900713          	li	a4,9
     54c:	04c76263          	bltu	a4,a2,590 <getint+0x6c>
    if (i > INT_MAX / 10 || **s - '0' > INT_MAX - 10 * i)
     550:	0cccd737          	lui	a4,0xcccd
     554:	ccc70713          	addi	a4,a4,-820 # ccccccc <seed+0xcccb694>
     558:	fca74ce3          	blt	a4,a0,530 <getint+0xc>
     55c:	fd06869b          	addiw	a3,a3,-48
     560:	0025161b          	slliw	a2,a0,0x2
     564:	00a6063b          	addw	a2,a2,a0
     568:	0016161b          	slliw	a2,a2,0x1
     56c:	40c0073b          	negw	a4,a2
     570:	800005b7          	lui	a1,0x80000
     574:	fff5859b          	addiw	a1,a1,-1 # 7fffffff <seed+0x7fffe9c7>
     578:	40c5863b          	subw	a2,a1,a2
     57c:	00d64663          	blt	a2,a3,588 <getint+0x64>
    else
      i = 10 * i + (**s - '0');
     580:	40e6853b          	subw	a0,a3,a4
     584:	fb1ff06f          	j	534 <getint+0x10>
      i = -1;
     588:	fff00513          	li	a0,-1
     58c:	fa9ff06f          	j	534 <getint+0x10>
  }
  return i;
}
     590:	00008067          	ret

0000000000000594 <pad>:
  if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w)
     594:	000127b7          	lui	a5,0x12
     598:	00f77733          	and	a4,a4,a5
     59c:	08071063          	bnez	a4,61c <pad+0x88>
static void pad(FILE *f, char c, size_t w, size_t l, int fl) {
     5a0:	ee010113          	addi	sp,sp,-288
     5a4:	10113c23          	sd	ra,280(sp)
     5a8:	10913423          	sd	s1,264(sp)
     5ac:	00050493          	mv	s1,a0
  if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w)
     5b0:	00c6ea63          	bltu	a3,a2,5c4 <pad+0x30>
}
     5b4:	11813083          	ld	ra,280(sp)
     5b8:	10813483          	ld	s1,264(sp)
     5bc:	12010113          	addi	sp,sp,288
     5c0:	00008067          	ret
     5c4:	10813823          	sd	s0,272(sp)
  l = w - l;
     5c8:	40d60433          	sub	s0,a2,a3
  memset(pad, c, l > sizeof pad ? sizeof pad : l);
     5cc:	00040613          	mv	a2,s0
     5d0:	10000793          	li	a5,256
     5d4:	0087f463          	bgeu	a5,s0,5dc <pad+0x48>
     5d8:	00078613          	mv	a2,a5
     5dc:	00010513          	mv	a0,sp
     5e0:	c45ff0ef          	jal	224 <memset>
  for (; l >= sizeof pad; l -= sizeof pad)
     5e4:	0180006f          	j	5fc <pad+0x68>
    out(f, pad, sizeof pad);
     5e8:	10000613          	li	a2,256
     5ec:	00010593          	mv	a1,sp
     5f0:	00048513          	mv	a0,s1
     5f4:	e55ff0ef          	jal	448 <out>
  for (; l >= sizeof pad; l -= sizeof pad)
     5f8:	f0040413          	addi	s0,s0,-256
     5fc:	0ff00793          	li	a5,255
     600:	fe87e4e3          	bltu	a5,s0,5e8 <pad+0x54>
  out(f, pad, l);
     604:	00040613          	mv	a2,s0
     608:	00010593          	mv	a1,sp
     60c:	00048513          	mv	a0,s1
     610:	e39ff0ef          	jal	448 <out>
     614:	11013403          	ld	s0,272(sp)
     618:	f9dff06f          	j	5b4 <pad+0x20>
     61c:	00008067          	ret

0000000000000620 <printf_core>:

// theoretically you can implement all other *printf functions using this one...
static int printf_core(FILE *f, const char *fmt, va_list *ap, union arg *nl_arg, int *nl_type) {
     620:	f4010113          	addi	sp,sp,-192
     624:	0a113c23          	sd	ra,184(sp)
     628:	0a813823          	sd	s0,176(sp)
     62c:	0a913423          	sd	s1,168(sp)
     630:	0b213023          	sd	s2,160(sp)
     634:	09313c23          	sd	s3,152(sp)
     638:	09413823          	sd	s4,144(sp)
     63c:	09513423          	sd	s5,136(sp)
     640:	09613023          	sd	s6,128(sp)
     644:	07713c23          	sd	s7,120(sp)
     648:	07813823          	sd	s8,112(sp)
     64c:	07913423          	sd	s9,104(sp)
     650:	07a13023          	sd	s10,96(sp)
     654:	05b13c23          	sd	s11,88(sp)
     658:	00050b13          	mv	s6,a0
     65c:	00060d93          	mv	s11,a2
     660:	00d13823          	sd	a3,16(sp)
     664:	00e13c23          	sd	a4,24(sp)
  char *a, *z, *s = (char *)fmt;
     668:	04b13423          	sd	a1,72(sp)
  unsigned l10n = 0, fl;
  int w, p, xp;
  union arg arg;
  int argpos;
  unsigned st, ps;
  int cnt = 0, l = 0;
     66c:	00000413          	li	s0,0
     670:	00000a93          	li	s5,0
  unsigned l10n = 0, fl;
     674:	00013023          	sd	zero,0(sp)
     678:	0780006f          	j	6f0 <printf_core+0xd0>
    cnt += l;
    if (!*s)
      break;

    /* Handle literal text and %% format specifiers */
    for (a = s; *s && *s != '%'; s++)
     67c:	00140413          	addi	s0,s0,1
     680:	04813423          	sd	s0,72(sp)
     684:	04813403          	ld	s0,72(sp)
     688:	00044783          	lbu	a5,0(s0)
     68c:	00078663          	beqz	a5,698 <printf_core+0x78>
     690:	02500713          	li	a4,37
     694:	fee794e3          	bne	a5,a4,67c <printf_core+0x5c>
      ;
    for (z = s; s[0] == '%' && s[1] == '%'; z++, s += 2)
     698:	04813783          	ld	a5,72(sp)
     69c:	0007c683          	lbu	a3,0(a5) # 12000 <seed+0x109c8>
     6a0:	02500713          	li	a4,37
     6a4:	00e69e63          	bne	a3,a4,6c0 <printf_core+0xa0>
     6a8:	0017c683          	lbu	a3,1(a5)
     6ac:	00e69a63          	bne	a3,a4,6c0 <printf_core+0xa0>
     6b0:	00140413          	addi	s0,s0,1
     6b4:	00278793          	addi	a5,a5,2
     6b8:	04f13423          	sd	a5,72(sp)
     6bc:	fddff06f          	j	698 <printf_core+0x78>
      ;
    if (z - a > INT_MAX - cnt)
     6c0:	41440433          	sub	s0,s0,s4
     6c4:	800009b7          	lui	s3,0x80000
     6c8:	fff9899b          	addiw	s3,s3,-1 # 7fffffff <seed+0x7fffe9c7>
     6cc:	417989bb          	subw	s3,s3,s7
     6d0:	7889c063          	blt	s3,s0,e50 <printf_core+0x830>
      goto overflow;
    l = z - a;
     6d4:	0004041b          	sext.w	s0,s0
    if (f)
     6d8:	000b0a63          	beqz	s6,6ec <printf_core+0xcc>
      out(f, a, l);
     6dc:	00040613          	mv	a2,s0
     6e0:	000a0593          	mv	a1,s4
     6e4:	000b0513          	mv	a0,s6
     6e8:	d61ff0ef          	jal	448 <out>
    if (l)
     6ec:	02040e63          	beqz	s0,728 <printf_core+0x108>
    if (l > INT_MAX - cnt)
     6f0:	800007b7          	lui	a5,0x80000
     6f4:	fff7879b          	addiw	a5,a5,-1 # 7fffffff <seed+0x7fffe9c7>
     6f8:	415787bb          	subw	a5,a5,s5
     6fc:	7487c663          	blt	a5,s0,e48 <printf_core+0x828>
    cnt += l;
     700:	008a8bbb          	addw	s7,s5,s0
     704:	000b8a93          	mv	s5,s7
    if (!*s)
     708:	04813a03          	ld	s4,72(sp)
     70c:	000a4783          	lbu	a5,0(s4)
     710:	f6079ae3          	bnez	a5,684 <printf_core+0x64>
    pad(f, ' ', w, pl + p, fl ^ LEFT_ADJ);

    l = w;
  }

  if (f)
     714:	760b1063          	bnez	s6,e74 <printf_core+0x854>
    return cnt;
  if (!l10n)
     718:	00013783          	ld	a5,0(sp)
     71c:	7c078463          	beqz	a5,ee4 <printf_core+0x8c4>
    return 0;

  for (i = 1; i <= NL_ARGMAX && nl_type[i]; i++)
     720:	00100413          	li	s0,1
     724:	6d40006f          	j	df8 <printf_core+0x7d8>
    if (isdigit(s[1]) && s[2] == '$') {
     728:	04813783          	ld	a5,72(sp)
     72c:	0017cc83          	lbu	s9,1(a5)
     730:	fd0c869b          	addiw	a3,s9,-48
     734:	00900713          	li	a4,9
     738:	00d76863          	bltu	a4,a3,748 <printf_core+0x128>
     73c:	0027c683          	lbu	a3,2(a5)
     740:	02400713          	li	a4,36
     744:	00e68c63          	beq	a3,a4,75c <printf_core+0x13c>
      s++;
     748:	00178793          	addi	a5,a5,1
     74c:	04f13423          	sd	a5,72(sp)
      argpos = -1;
     750:	fff00c93          	li	s9,-1
    for (fl = 0; (unsigned)(*s - ' ') < 32 && (FLAGMASK & (1U << (*s - ' '))); s++)
     754:	00000493          	li	s1,0
     758:	0300006f          	j	788 <printf_core+0x168>
      argpos = s[1] - '0';
     75c:	fd0c8c9b          	addiw	s9,s9,-48
      s += 3;
     760:	00378793          	addi	a5,a5,3
     764:	04f13423          	sd	a5,72(sp)
      l10n = 1;
     768:	00100793          	li	a5,1
     76c:	00f13023          	sd	a5,0(sp)
      s += 3;
     770:	fe5ff06f          	j	754 <printf_core+0x134>
      fl |= 1U << (*s - ' ');
     774:	00100793          	li	a5,1
     778:	00d797bb          	sllw	a5,a5,a3
     77c:	0097e4b3          	or	s1,a5,s1
    for (fl = 0; (unsigned)(*s - ' ') < 32 && (FLAGMASK & (1U << (*s - ' '))); s++)
     780:	00170713          	addi	a4,a4,1
     784:	04e13423          	sd	a4,72(sp)
     788:	04813703          	ld	a4,72(sp)
     78c:	00074603          	lbu	a2,0(a4)
     790:	fe06069b          	addiw	a3,a2,-32
     794:	01f00793          	li	a5,31
     798:	00d7ec63          	bltu	a5,a3,7b0 <printf_core+0x190>
     79c:	000137b7          	lui	a5,0x13
     7a0:	8097879b          	addiw	a5,a5,-2039 # 12809 <seed+0x111d1>
     7a4:	00d7d7bb          	srlw	a5,a5,a3
     7a8:	0017f793          	andi	a5,a5,1
     7ac:	fc0794e3          	bnez	a5,774 <printf_core+0x154>
    if (*s == '*') {
     7b0:	02a00793          	li	a5,42
     7b4:	0af61a63          	bne	a2,a5,868 <printf_core+0x248>
      if (isdigit(s[1]) && s[2] == '$') {
     7b8:	00174783          	lbu	a5,1(a4)
     7bc:	fd07861b          	addiw	a2,a5,-48
     7c0:	00900693          	li	a3,9
     7c4:	00c6e863          	bltu	a3,a2,7d4 <printf_core+0x1b4>
     7c8:	00274683          	lbu	a3,2(a4)
     7cc:	02400713          	li	a4,36
     7d0:	04e68063          	beq	a3,a4,810 <printf_core+0x1f0>
      } else if (!l10n) {
     7d4:	00013783          	ld	a5,0(sp)
     7d8:	68079063          	bnez	a5,e58 <printf_core+0x838>
        w = f ? va_arg(*ap, int) : 0;
     7dc:	080b0263          	beqz	s6,860 <printf_core+0x240>
     7e0:	000db783          	ld	a5,0(s11)
     7e4:	00878713          	addi	a4,a5,8
     7e8:	00edb023          	sd	a4,0(s11)
     7ec:	0007ac03          	lw	s8,0(a5)
        s++;
     7f0:	04813783          	ld	a5,72(sp)
     7f4:	00178793          	addi	a5,a5,1
     7f8:	04f13423          	sd	a5,72(sp)
      if (w < 0)
     7fc:	060c5e63          	bgez	s8,878 <printf_core+0x258>
        fl |= LEFT_ADJ, w = -w;
     800:	000027b7          	lui	a5,0x2
     804:	00f4e4b3          	or	s1,s1,a5
     808:	41800c3b          	negw	s8,s8
     80c:	06c0006f          	j	878 <printf_core+0x258>
        if (!f)
     810:	020b0863          	beqz	s6,840 <printf_core+0x220>
          w = nl_arg[s[1] - '0'].i;
     814:	00379793          	slli	a5,a5,0x3
     818:	e8078793          	addi	a5,a5,-384 # 1e80 <seed+0x848>
     81c:	01013703          	ld	a4,16(sp)
     820:	00f707b3          	add	a5,a4,a5
     824:	0007ac03          	lw	s8,0(a5)
        s += 3;
     828:	04813783          	ld	a5,72(sp)
     82c:	00378793          	addi	a5,a5,3
     830:	04f13423          	sd	a5,72(sp)
        l10n = 1;
     834:	00100793          	li	a5,1
     838:	00f13023          	sd	a5,0(sp)
        s += 3;
     83c:	fc1ff06f          	j	7fc <printf_core+0x1dc>
          nl_type[s[1] - '0'] = INT, w = 0;
     840:	00279793          	slli	a5,a5,0x2
     844:	f4078793          	addi	a5,a5,-192
     848:	01813703          	ld	a4,24(sp)
     84c:	00f707b3          	add	a5,a4,a5
     850:	00900713          	li	a4,9
     854:	00e7a023          	sw	a4,0(a5)
     858:	00040c13          	mv	s8,s0
     85c:	fcdff06f          	j	828 <printf_core+0x208>
        w = f ? va_arg(*ap, int) : 0;
     860:	00040c13          	mv	s8,s0
     864:	f8dff06f          	j	7f0 <printf_core+0x1d0>
    } else if ((w = getint(&s)) < 0)
     868:	04810513          	addi	a0,sp,72
     86c:	cb9ff0ef          	jal	524 <getint>
     870:	00050c13          	mv	s8,a0
     874:	5e054663          	bltz	a0,e60 <printf_core+0x840>
    if (*s == '.' && s[1] == '*') {
     878:	04813783          	ld	a5,72(sp)
     87c:	0007c703          	lbu	a4,0(a5)
     880:	02e00693          	li	a3,46
     884:	0ad71a63          	bne	a4,a3,938 <printf_core+0x318>
     888:	0017c603          	lbu	a2,1(a5)
     88c:	02a00693          	li	a3,42
     890:	0ad61463          	bne	a2,a3,938 <printf_core+0x318>
      if (isdigit(s[2]) && s[3] == '$') {
     894:	0027c703          	lbu	a4,2(a5)
     898:	fd07061b          	addiw	a2,a4,-48
     89c:	00900693          	li	a3,9
     8a0:	00c6e863          	bltu	a3,a2,8b0 <printf_core+0x290>
     8a4:	0037c683          	lbu	a3,3(a5)
     8a8:	02400793          	li	a5,36
     8ac:	02f68e63          	beq	a3,a5,8e8 <printf_core+0x2c8>
      } else if (!l10n) {
     8b0:	00013783          	ld	a5,0(sp)
     8b4:	5a079a63          	bnez	a5,e68 <printf_core+0x848>
        p = f ? va_arg(*ap, int) : 0;
     8b8:	060b0c63          	beqz	s6,930 <printf_core+0x310>
     8bc:	000db783          	ld	a5,0(s11)
     8c0:	00878713          	addi	a4,a5,8
     8c4:	00edb023          	sd	a4,0(s11)
     8c8:	0007ad03          	lw	s10,0(a5)
        s += 2;
     8cc:	04813783          	ld	a5,72(sp)
     8d0:	00278793          	addi	a5,a5,2
     8d4:	04f13423          	sd	a5,72(sp)
      xp = (p >= 0);
     8d8:	fffd4793          	not	a5,s10
     8dc:	01f7d79b          	srliw	a5,a5,0x1f
     8e0:	00f13423          	sd	a5,8(sp)
     8e4:	0640006f          	j	948 <printf_core+0x328>
        if (!f)
     8e8:	020b0463          	beqz	s6,910 <printf_core+0x2f0>
          p = nl_arg[s[2] - '0'].i;
     8ec:	00371793          	slli	a5,a4,0x3
     8f0:	e8078793          	addi	a5,a5,-384
     8f4:	01013703          	ld	a4,16(sp)
     8f8:	00f707b3          	add	a5,a4,a5
     8fc:	0007ad03          	lw	s10,0(a5)
        s += 4;
     900:	04813783          	ld	a5,72(sp)
     904:	00478793          	addi	a5,a5,4
     908:	04f13423          	sd	a5,72(sp)
     90c:	fcdff06f          	j	8d8 <printf_core+0x2b8>
          nl_type[s[2] - '0'] = INT, p = 0;
     910:	00271793          	slli	a5,a4,0x2
     914:	f4078793          	addi	a5,a5,-192
     918:	01813703          	ld	a4,24(sp)
     91c:	00f707b3          	add	a5,a4,a5
     920:	00900713          	li	a4,9
     924:	00e7a023          	sw	a4,0(a5)
     928:	00040d13          	mv	s10,s0
     92c:	fd5ff06f          	j	900 <printf_core+0x2e0>
        p = f ? va_arg(*ap, int) : 0;
     930:	00040d13          	mv	s10,s0
     934:	f99ff06f          	j	8cc <printf_core+0x2ac>
    } else if (*s == '.') {
     938:	02e00693          	li	a3,46
     93c:	00d70a63          	beq	a4,a3,950 <printf_core+0x330>
      xp = 0;
     940:	00813423          	sd	s0,8(sp)
      p = -1;
     944:	fff00d13          	li	s10,-1
    st = 0;
     948:	00000913          	li	s2,0
     94c:	0280006f          	j	974 <printf_core+0x354>
      s++;
     950:	00178793          	addi	a5,a5,1
     954:	04f13423          	sd	a5,72(sp)
      p = getint(&s);
     958:	04810513          	addi	a0,sp,72
     95c:	bc9ff0ef          	jal	524 <getint>
     960:	00050d13          	mv	s10,a0
      xp = 1;
     964:	00100793          	li	a5,1
     968:	00f13423          	sd	a5,8(sp)
     96c:	fddff06f          	j	948 <printf_core+0x328>
      st = states[st] S(*s++);
     970:	00078913          	mv	s2,a5
      if (OOB(*s))
     974:	04813703          	ld	a4,72(sp)
     978:	00074783          	lbu	a5,0(a4)
     97c:	fbf7879b          	addiw	a5,a5,-65
     980:	03900693          	li	a3,57
     984:	4ef6e663          	bltu	a3,a5,e70 <printf_core+0x850>
      st = states[st] S(*s++);
     988:	00170793          	addi	a5,a4,1
     98c:	04f13423          	sd	a5,72(sp)
     990:	00074683          	lbu	a3,0(a4)
     994:	fbf6869b          	addiw	a3,a3,-65
     998:	02091713          	slli	a4,s2,0x20
     99c:	02075713          	srli	a4,a4,0x20
     9a0:	00371793          	slli	a5,a4,0x3
     9a4:	40e787b3          	sub	a5,a5,a4
     9a8:	00279793          	slli	a5,a5,0x2
     9ac:	00e787b3          	add	a5,a5,a4
     9b0:	00179793          	slli	a5,a5,0x1
     9b4:	00001717          	auipc	a4,0x1
     9b8:	80c70713          	addi	a4,a4,-2036 # 11c0 <states>
     9bc:	00f707b3          	add	a5,a4,a5
     9c0:	00d787b3          	add	a5,a5,a3
     9c4:	0007c583          	lbu	a1,0(a5)
     9c8:	0005879b          	sext.w	a5,a1
    } while (st - 1 < STOP);
     9cc:	fff5869b          	addiw	a3,a1,-1
     9d0:	00600713          	li	a4,6
     9d4:	f8d77ee3          	bgeu	a4,a3,970 <printf_core+0x350>
    if (!st)
     9d8:	4c078e63          	beqz	a5,eb4 <printf_core+0x894>
    if (st == NOARG) {
     9dc:	01800713          	li	a4,24
     9e0:	02e78263          	beq	a5,a4,a04 <printf_core+0x3e4>
      if (argpos >= 0) {
     9e4:	080cc663          	bltz	s9,a70 <printf_core+0x450>
        if (!f)
     9e8:	060b0a63          	beqz	s6,a5c <printf_core+0x43c>
          arg = nl_arg[argpos];
     9ec:	003c9793          	slli	a5,s9,0x3
     9f0:	01013703          	ld	a4,16(sp)
     9f4:	00f707b3          	add	a5,a4,a5
     9f8:	0007b783          	ld	a5,0(a5)
     9fc:	04f13023          	sd	a5,64(sp)
     a00:	0080006f          	j	a08 <printf_core+0x3e8>
      if (argpos >= 0)
     a04:	4a0cdc63          	bgez	s9,ebc <printf_core+0x89c>
    if (!f)
     a08:	ce0b04e3          	beqz	s6,6f0 <printf_core+0xd0>
    t = s[-1];
     a0c:	04813783          	ld	a5,72(sp)
     a10:	fff7c783          	lbu	a5,-1(a5)
     a14:	00078c9b          	sext.w	s9,a5
    if (fl & LEFT_ADJ)
     a18:	00002737          	lui	a4,0x2
     a1c:	00e4f733          	and	a4,s1,a4
     a20:	00070863          	beqz	a4,a30 <printf_core+0x410>
      fl &= ~ZERO_PAD;
     a24:	ffff0737          	lui	a4,0xffff0
     a28:	fff70713          	addi	a4,a4,-1 # fffffffffffeffff <seed+0xfffffffffffee9c7>
     a2c:	00e4f4b3          	and	s1,s1,a4
    switch (t) {
     a30:	fa87879b          	addiw	a5,a5,-88
     a34:	0ff7f693          	zext.b	a3,a5
     a38:	02000713          	li	a4,32
     a3c:	2cd76863          	bltu	a4,a3,d0c <printf_core+0x6ec>
     a40:	00269793          	slli	a5,a3,0x2
     a44:	00000717          	auipc	a4,0x0
     a48:	6cc70713          	addi	a4,a4,1740 # 1110 <__muldi3+0xa4>
     a4c:	00e787b3          	add	a5,a5,a4
     a50:	0007a783          	lw	a5,0(a5)
     a54:	00e787b3          	add	a5,a5,a4
     a58:	00078067          	jr	a5
          nl_type[argpos] = st;
     a5c:	002c9793          	slli	a5,s9,0x2
     a60:	01813703          	ld	a4,24(sp)
     a64:	00f707b3          	add	a5,a4,a5
     a68:	00b7a023          	sw	a1,0(a5)
     a6c:	f9dff06f          	j	a08 <printf_core+0x3e8>
      } else if (f)
     a70:	440b0a63          	beqz	s6,ec4 <printf_core+0x8a4>
        pop_arg(&arg, st, ap);
     a74:	000d8613          	mv	a2,s11
     a78:	04010513          	addi	a0,sp,64
     a7c:	821ff0ef          	jal	29c <pop_arg>
     a80:	f89ff06f          	j	a08 <printf_core+0x3e8>
        switch (ps) {
     a84:	00600793          	li	a5,6
     a88:	c727e4e3          	bltu	a5,s2,6f0 <printf_core+0xd0>
     a8c:	00291793          	slli	a5,s2,0x2
     a90:	00000717          	auipc	a4,0x0
     a94:	70470713          	addi	a4,a4,1796 # 1194 <__muldi3+0x128>
     a98:	00e787b3          	add	a5,a5,a4
     a9c:	0007a783          	lw	a5,0(a5)
     aa0:	00e787b3          	add	a5,a5,a4
     aa4:	00078067          	jr	a5
            *(int *)arg.p = cnt;
     aa8:	04013783          	ld	a5,64(sp)
     aac:	0177a023          	sw	s7,0(a5)
            break;
     ab0:	c41ff06f          	j	6f0 <printf_core+0xd0>
            *(long *)arg.p = cnt;
     ab4:	04013783          	ld	a5,64(sp)
     ab8:	0177b023          	sd	s7,0(a5)
            break;
     abc:	c35ff06f          	j	6f0 <printf_core+0xd0>
            *(long long *)arg.p = cnt;
     ac0:	04013783          	ld	a5,64(sp)
     ac4:	0177b023          	sd	s7,0(a5)
            break;
     ac8:	c29ff06f          	j	6f0 <printf_core+0xd0>
            *(unsigned short *)arg.p = cnt;
     acc:	04013783          	ld	a5,64(sp)
     ad0:	01779023          	sh	s7,0(a5)
            break;
     ad4:	c1dff06f          	j	6f0 <printf_core+0xd0>
            *(unsigned char *)arg.p = cnt;
     ad8:	04013783          	ld	a5,64(sp)
     adc:	01778023          	sb	s7,0(a5)
            break;
     ae0:	c11ff06f          	j	6f0 <printf_core+0xd0>
            *(size_t *)arg.p = cnt;
     ae4:	04013783          	ld	a5,64(sp)
     ae8:	0177b023          	sd	s7,0(a5)
            break;
     aec:	c05ff06f          	j	6f0 <printf_core+0xd0>
            *(uintmax_t *)arg.p = cnt;
     af0:	04013783          	ld	a5,64(sp)
     af4:	0177b023          	sd	s7,0(a5)
        continue;
     af8:	bf9ff06f          	j	6f0 <printf_core+0xd0>
        p = MAX((size_t)p, 2 * sizeof(void *));
     afc:	01000793          	li	a5,16
     b00:	00fd7463          	bgeu	s10,a5,b08 <printf_core+0x4e8>
     b04:	00078d13          	mv	s10,a5
     b08:	000d0d1b          	sext.w	s10,s10
        fl |= ALT_FORM;
     b0c:	0084e493          	ori	s1,s1,8
        t = 'x';
     b10:	07800c93          	li	s9,120
        a = fmt_x(arg.i, z, t & 32);
     b14:	020cf613          	andi	a2,s9,32
     b18:	04010593          	addi	a1,sp,64
     b1c:	04013503          	ld	a0,64(sp)
     b20:	945ff0ef          	jal	464 <fmt_x>
     b24:	00050a13          	mv	s4,a0
        if (arg.i && (fl & ALT_FORM))
     b28:	04013783          	ld	a5,64(sp)
     b2c:	10078e63          	beqz	a5,c48 <printf_core+0x628>
     b30:	0084f793          	andi	a5,s1,8
     b34:	12078063          	beqz	a5,c54 <printf_core+0x634>
          prefix += (t >> 4), pl = 2;
     b38:	404cdc9b          	sraiw	s9,s9,0x4
     b3c:	00000797          	auipc	a5,0x0
     b40:	57c78793          	addi	a5,a5,1404 # 10b8 <__muldi3+0x4c>
     b44:	01978cb3          	add	s9,a5,s9
     b48:	00200413          	li	s0,2
     b4c:	0980006f          	j	be4 <printf_core+0x5c4>
            a = fmt_o(arg.i, z);
     b50:	04010593          	addi	a1,sp,64
     b54:	04013503          	ld	a0,64(sp)
     b58:	945ff0ef          	jal	49c <fmt_o>
     b5c:	00050a13          	mv	s4,a0
            if ((fl & ALT_FORM) && p < z - a + 1)
     b60:	0084f793          	andi	a5,s1,8
     b64:	0e078e63          	beqz	a5,c60 <printf_core+0x640>
     b68:	04010793          	addi	a5,sp,64
     b6c:	40a787b3          	sub	a5,a5,a0
     b70:	0fa7ce63          	blt	a5,s10,c6c <printf_core+0x64c>
              p = z - a + 1;
     b74:	00178d1b          	addiw	s10,a5,1
    prefix = "-+   0X0x";
     b78:	00000c97          	auipc	s9,0x0
     b7c:	540c8c93          	addi	s9,s9,1344 # 10b8 <__muldi3+0x4c>
     b80:	0640006f          	j	be4 <printf_core+0x5c4>
            if (arg.i > INTMAX_MAX) {
     b84:	04013783          	ld	a5,64(sp)
     b88:	0207c663          	bltz	a5,bb4 <printf_core+0x594>
            } else if (fl & MARK_POS) {
     b8c:	000017b7          	lui	a5,0x1
     b90:	80078793          	addi	a5,a5,-2048 # 800 <printf_core+0x1e0>
     b94:	00f4f7b3          	and	a5,s1,a5
     b98:	08079a63          	bnez	a5,c2c <printf_core+0x60c>
            } else if (fl & PAD_POS) {
     b9c:	0014f793          	andi	a5,s1,1
     ba0:	08078e63          	beqz	a5,c3c <printf_core+0x61c>
            pl = 1;
     ba4:	00100413          	li	s0,1
              prefix += 2;
     ba8:	00000c97          	auipc	s9,0x0
     bac:	512c8c93          	addi	s9,s9,1298 # 10ba <__muldi3+0x4e>
     bb0:	0240006f          	j	bd4 <printf_core+0x5b4>
              arg.i = -arg.i;
     bb4:	40f007b3          	neg	a5,a5
     bb8:	04f13023          	sd	a5,64(sp)
            pl = 1;
     bbc:	00100413          	li	s0,1
    prefix = "-+   0X0x";
     bc0:	00000c97          	auipc	s9,0x0
     bc4:	4f8c8c93          	addi	s9,s9,1272 # 10b8 <__muldi3+0x4c>
     bc8:	00c0006f          	j	bd4 <printf_core+0x5b4>
    switch (t) {
     bcc:	00000c97          	auipc	s9,0x0
     bd0:	4ecc8c93          	addi	s9,s9,1260 # 10b8 <__muldi3+0x4c>
            a = fmt_u(arg.i, z);
     bd4:	04010593          	addi	a1,sp,64
     bd8:	04013503          	ld	a0,64(sp)
     bdc:	8e9ff0ef          	jal	4c4 <fmt_u>
     be0:	00050a13          	mv	s4,a0
        if (xp && p < 0)
     be4:	00813783          	ld	a5,8(sp)
     be8:	00078a63          	beqz	a5,bfc <printf_core+0x5dc>
     bec:	2e0d4063          	bltz	s10,ecc <printf_core+0x8ac>
          fl &= ~ZERO_PAD;
     bf0:	ffff07b7          	lui	a5,0xffff0
     bf4:	fff78793          	addi	a5,a5,-1 # fffffffffffeffff <seed+0xfffffffffffee9c7>
     bf8:	00f4f4b3          	and	s1,s1,a5
        if (!arg.i && !p) {
     bfc:	04013703          	ld	a4,64(sp)
     c00:	00071463          	bnez	a4,c08 <printf_core+0x5e8>
     c04:	1c0d0863          	beqz	s10,dd4 <printf_core+0x7b4>
        p = MAX(p, z - a + !arg.i);
     c08:	04010793          	addi	a5,sp,64
     c0c:	414787b3          	sub	a5,a5,s4
     c10:	00173713          	seqz	a4,a4
     c14:	00e787b3          	add	a5,a5,a4
     c18:	00fd5463          	bge	s10,a5,c20 <printf_core+0x600>
     c1c:	00078d13          	mv	s10,a5
     c20:	000d0d1b          	sext.w	s10,s10
    z = buf + sizeof(buf);
     c24:	04010913          	addi	s2,sp,64
        break;
     c28:	0f00006f          	j	d18 <printf_core+0x6f8>
            pl = 1;
     c2c:	00100413          	li	s0,1
              prefix++;
     c30:	00000c97          	auipc	s9,0x0
     c34:	489c8c93          	addi	s9,s9,1161 # 10b9 <__muldi3+0x4d>
     c38:	f9dff06f          	j	bd4 <printf_core+0x5b4>
    prefix = "-+   0X0x";
     c3c:	00000c97          	auipc	s9,0x0
     c40:	47cc8c93          	addi	s9,s9,1148 # 10b8 <__muldi3+0x4c>
     c44:	f91ff06f          	j	bd4 <printf_core+0x5b4>
     c48:	00000c97          	auipc	s9,0x0
     c4c:	470c8c93          	addi	s9,s9,1136 # 10b8 <__muldi3+0x4c>
     c50:	f95ff06f          	j	be4 <printf_core+0x5c4>
     c54:	00000c97          	auipc	s9,0x0
     c58:	464c8c93          	addi	s9,s9,1124 # 10b8 <__muldi3+0x4c>
     c5c:	f89ff06f          	j	be4 <printf_core+0x5c4>
     c60:	00000c97          	auipc	s9,0x0
     c64:	458c8c93          	addi	s9,s9,1112 # 10b8 <__muldi3+0x4c>
     c68:	f7dff06f          	j	be4 <printf_core+0x5c4>
     c6c:	00000c97          	auipc	s9,0x0
     c70:	44cc8c93          	addi	s9,s9,1100 # 10b8 <__muldi3+0x4c>
     c74:	f71ff06f          	j	be4 <printf_core+0x5c4>
        *(a = z - (p = 1)) = arg.i;
     c78:	04013783          	ld	a5,64(sp)
     c7c:	02f10fa3          	sb	a5,63(sp)
        fl &= ~ZERO_PAD;
     c80:	ffff07b7          	lui	a5,0xffff0
     c84:	fff78793          	addi	a5,a5,-1 # fffffffffffeffff <seed+0xfffffffffffee9c7>
     c88:	00f4f4b3          	and	s1,s1,a5
    prefix = "-+   0X0x";
     c8c:	00000c97          	auipc	s9,0x0
     c90:	42cc8c93          	addi	s9,s9,1068 # 10b8 <__muldi3+0x4c>
        *(a = z - (p = 1)) = arg.i;
     c94:	00100d13          	li	s10,1
    z = buf + sizeof(buf);
     c98:	04010913          	addi	s2,sp,64
        *(a = z - (p = 1)) = arg.i;
     c9c:	03f10a13          	addi	s4,sp,63
        break;
     ca0:	0780006f          	j	d18 <printf_core+0x6f8>
        a = arg.p ? arg.p : "(null)";
     ca4:	04013a03          	ld	s4,64(sp)
     ca8:	020a0e63          	beqz	s4,ce4 <printf_core+0x6c4>
        z = a + strnlen(a, p < 0 ? INT_MAX : p);
     cac:	040d4263          	bltz	s10,cf0 <printf_core+0x6d0>
     cb0:	000d0593          	mv	a1,s10
     cb4:	000a0513          	mv	a0,s4
     cb8:	db8ff0ef          	jal	270 <strnlen>
     cbc:	00050793          	mv	a5,a0
     cc0:	00aa0933          	add	s2,s4,a0
        if (p < 0 && *z)
     cc4:	020d4c63          	bltz	s10,cfc <printf_core+0x6dc>
        p = z - a;
     cc8:	00078d1b          	sext.w	s10,a5
        fl &= ~ZERO_PAD;
     ccc:	ffff07b7          	lui	a5,0xffff0
     cd0:	fff78793          	addi	a5,a5,-1 # fffffffffffeffff <seed+0xfffffffffffee9c7>
     cd4:	00f4f4b3          	and	s1,s1,a5
    prefix = "-+   0X0x";
     cd8:	00000c97          	auipc	s9,0x0
     cdc:	3e0c8c93          	addi	s9,s9,992 # 10b8 <__muldi3+0x4c>
        break;
     ce0:	0380006f          	j	d18 <printf_core+0x6f8>
        a = arg.p ? arg.p : "(null)";
     ce4:	00000a17          	auipc	s4,0x0
     ce8:	3e4a0a13          	addi	s4,s4,996 # 10c8 <__muldi3+0x5c>
     cec:	fc1ff06f          	j	cac <printf_core+0x68c>
        z = a + strnlen(a, p < 0 ? INT_MAX : p);
     cf0:	800005b7          	lui	a1,0x80000
     cf4:	fff5c593          	not	a1,a1
     cf8:	fbdff06f          	j	cb4 <printf_core+0x694>
        if (p < 0 && *z)
     cfc:	00094703          	lbu	a4,0(s2)
     d00:	fc0704e3          	beqz	a4,cc8 <printf_core+0x6a8>
inval:
  // errno = EINVAL;
  // return -1;
overflow:
  // errno = EOVERFLOW;
  return -1;
     d04:	fff00a93          	li	s5,-1
     d08:	16c0006f          	j	e74 <printf_core+0x854>
    switch (t) {
     d0c:	00000c97          	auipc	s9,0x0
     d10:	3acc8c93          	addi	s9,s9,940 # 10b8 <__muldi3+0x4c>
     d14:	04010913          	addi	s2,sp,64
    if (p < z - a)
     d18:	41490933          	sub	s2,s2,s4
     d1c:	012d5463          	bge	s10,s2,d24 <printf_core+0x704>
      p = z - a;
     d20:	00090d1b          	sext.w	s10,s2
    if (p > INT_MAX - pl)
     d24:	800007b7          	lui	a5,0x80000
     d28:	fff7879b          	addiw	a5,a5,-1 # 7fffffff <seed+0x7fffe9c7>
     d2c:	408787bb          	subw	a5,a5,s0
     d30:	1ba7c263          	blt	a5,s10,ed4 <printf_core+0x8b4>
    if (w < pl + p)
     d34:	008d0bbb          	addw	s7,s10,s0
     d38:	017c5463          	bge	s8,s7,d40 <printf_core+0x720>
      w = pl + p;
     d3c:	000b8c13          	mv	s8,s7
    if (w > INT_MAX - cnt)
     d40:	1989ce63          	blt	s3,s8,edc <printf_core+0x8bc>
    pad(f, ' ', w, pl + p, fl);
     d44:	00048713          	mv	a4,s1
     d48:	000b8693          	mv	a3,s7
     d4c:	000c0613          	mv	a2,s8
     d50:	02000593          	li	a1,32
     d54:	000b0513          	mv	a0,s6
     d58:	83dff0ef          	jal	594 <pad>
    out(f, prefix, pl);
     d5c:	00040613          	mv	a2,s0
     d60:	000c8593          	mv	a1,s9
     d64:	000b0513          	mv	a0,s6
     d68:	ee0ff0ef          	jal	448 <out>
    pad(f, '0', w, pl + p, fl ^ ZERO_PAD);
     d6c:	00010737          	lui	a4,0x10
     d70:	00e4c733          	xor	a4,s1,a4
     d74:	000b8693          	mv	a3,s7
     d78:	000c0613          	mv	a2,s8
     d7c:	03000593          	li	a1,48
     d80:	000b0513          	mv	a0,s6
     d84:	811ff0ef          	jal	594 <pad>
    pad(f, '0', p, z - a, 0);
     d88:	00000713          	li	a4,0
     d8c:	00090693          	mv	a3,s2
     d90:	000d0613          	mv	a2,s10
     d94:	03000593          	li	a1,48
     d98:	000b0513          	mv	a0,s6
     d9c:	ff8ff0ef          	jal	594 <pad>
    out(f, a, z - a);
     da0:	00090613          	mv	a2,s2
     da4:	000a0593          	mv	a1,s4
     da8:	000b0513          	mv	a0,s6
     dac:	e9cff0ef          	jal	448 <out>
    pad(f, ' ', w, pl + p, fl ^ LEFT_ADJ);
     db0:	00002737          	lui	a4,0x2
     db4:	00e4c733          	xor	a4,s1,a4
     db8:	000b8693          	mv	a3,s7
     dbc:	000c0613          	mv	a2,s8
     dc0:	02000593          	li	a1,32
     dc4:	000b0513          	mv	a0,s6
     dc8:	fccff0ef          	jal	594 <pad>
    l = w;
     dcc:	000c0413          	mv	s0,s8
     dd0:	921ff06f          	j	6f0 <printf_core+0xd0>
    z = buf + sizeof(buf);
     dd4:	04010913          	addi	s2,sp,64
          a = z;
     dd8:	00090a13          	mv	s4,s2
     ddc:	f3dff06f          	j	d18 <printf_core+0x6f8>
    pop_arg(nl_arg + i, nl_type[i], ap);
     de0:	00341513          	slli	a0,s0,0x3
     de4:	000d8613          	mv	a2,s11
     de8:	01013783          	ld	a5,16(sp)
     dec:	00a78533          	add	a0,a5,a0
     df0:	cacff0ef          	jal	29c <pop_arg>
  for (i = 1; i <= NL_ARGMAX && nl_type[i]; i++)
     df4:	00140413          	addi	s0,s0,1
     df8:	00900793          	li	a5,9
     dfc:	0287e063          	bltu	a5,s0,e1c <printf_core+0x7fc>
     e00:	00241793          	slli	a5,s0,0x2
     e04:	01813703          	ld	a4,24(sp)
     e08:	00f707b3          	add	a5,a4,a5
     e0c:	0007a583          	lw	a1,0(a5)
     e10:	fc0598e3          	bnez	a1,de0 <printf_core+0x7c0>
     e14:	0080006f          	j	e1c <printf_core+0x7fc>
  for (; i <= NL_ARGMAX && !nl_type[i]; i++)
     e18:	00140413          	addi	s0,s0,1
     e1c:	00900793          	li	a5,9
     e20:	0087ec63          	bltu	a5,s0,e38 <printf_core+0x818>
     e24:	00241793          	slli	a5,s0,0x2
     e28:	01813703          	ld	a4,24(sp)
     e2c:	00f707b3          	add	a5,a4,a5
     e30:	0007a783          	lw	a5,0(a5)
     e34:	fe0782e3          	beqz	a5,e18 <printf_core+0x7f8>
  if (i <= NL_ARGMAX)
     e38:	00900793          	li	a5,9
     e3c:	0a87f863          	bgeu	a5,s0,eec <printf_core+0x8cc>
  return 1;
     e40:	00100a93          	li	s5,1
     e44:	0300006f          	j	e74 <printf_core+0x854>
  return -1;
     e48:	fff00a93          	li	s5,-1
     e4c:	0280006f          	j	e74 <printf_core+0x854>
     e50:	fff00a93          	li	s5,-1
     e54:	0200006f          	j	e74 <printf_core+0x854>
     e58:	fff00a93          	li	s5,-1
     e5c:	0180006f          	j	e74 <printf_core+0x854>
     e60:	fff00a93          	li	s5,-1
     e64:	0100006f          	j	e74 <printf_core+0x854>
     e68:	fff00a93          	li	s5,-1
     e6c:	0080006f          	j	e74 <printf_core+0x854>
     e70:	fff00a93          	li	s5,-1
}
     e74:	000a8513          	mv	a0,s5
     e78:	0b813083          	ld	ra,184(sp)
     e7c:	0b013403          	ld	s0,176(sp)
     e80:	0a813483          	ld	s1,168(sp)
     e84:	0a013903          	ld	s2,160(sp)
     e88:	09813983          	ld	s3,152(sp)
     e8c:	09013a03          	ld	s4,144(sp)
     e90:	08813a83          	ld	s5,136(sp)
     e94:	08013b03          	ld	s6,128(sp)
     e98:	07813b83          	ld	s7,120(sp)
     e9c:	07013c03          	ld	s8,112(sp)
     ea0:	06813c83          	ld	s9,104(sp)
     ea4:	06013d03          	ld	s10,96(sp)
     ea8:	05813d83          	ld	s11,88(sp)
     eac:	0c010113          	addi	sp,sp,192
     eb0:	00008067          	ret
  return -1;
     eb4:	fff00a93          	li	s5,-1
     eb8:	fbdff06f          	j	e74 <printf_core+0x854>
     ebc:	fff00a93          	li	s5,-1
     ec0:	fb5ff06f          	j	e74 <printf_core+0x854>
        return 0;
     ec4:	00040a93          	mv	s5,s0
     ec8:	fadff06f          	j	e74 <printf_core+0x854>
  return -1;
     ecc:	fff00a93          	li	s5,-1
     ed0:	fa5ff06f          	j	e74 <printf_core+0x854>
     ed4:	fff00a93          	li	s5,-1
     ed8:	f9dff06f          	j	e74 <printf_core+0x854>
     edc:	fff00a93          	li	s5,-1
     ee0:	f95ff06f          	j	e74 <printf_core+0x854>
    return 0;
     ee4:	00000a93          	li	s5,0
     ee8:	f8dff06f          	j	e74 <printf_core+0x854>
  return -1;
     eec:	fff00a93          	li	s5,-1
     ef0:	f85ff06f          	j	e74 <printf_core+0x854>

0000000000000ef4 <vfprintf>:
  return ret;
}

#else

int vfprintf(FILE *restrict f, const char *restrict fmt, va_list ap) {
     ef4:	f5010113          	addi	sp,sp,-176
     ef8:	0a113423          	sd	ra,168(sp)
     efc:	0a813023          	sd	s0,160(sp)
     f00:	08913c23          	sd	s1,152(sp)
     f04:	00050413          	mv	s0,a0
     f08:	00058493          	mv	s1,a1
     f0c:	00c13423          	sd	a2,8(sp)
  int nl_type[NL_ARGMAX + 1] = {0};
     f10:	06013423          	sd	zero,104(sp)
     f14:	06013823          	sd	zero,112(sp)
     f18:	06013c23          	sd	zero,120(sp)
     f1c:	08013023          	sd	zero,128(sp)
     f20:	08013423          	sd	zero,136(sp)
  union arg nl_arg[NL_ARGMAX + 1];

  // preprocess nl arguments
  va_list ap2;
  va_copy(ap2, ap);
     f24:	00c13823          	sd	a2,16(sp)
  int ret = printf_core(0, fmt, &ap2, nl_arg, nl_type);
     f28:	06810713          	addi	a4,sp,104
     f2c:	01810693          	addi	a3,sp,24
     f30:	01010613          	addi	a2,sp,16
     f34:	00000513          	li	a0,0
     f38:	ee8ff0ef          	jal	620 <printf_core>
  va_end(ap2);

  if (ret < 0) {
     f3c:	00054e63          	bltz	a0,f58 <vfprintf+0x64>
    return ret;
  }
  return printf_core(f, fmt, &ap, nl_arg, nl_type);
     f40:	06810713          	addi	a4,sp,104
     f44:	01810693          	addi	a3,sp,24
     f48:	00810613          	addi	a2,sp,8
     f4c:	00048593          	mv	a1,s1
     f50:	00040513          	mv	a0,s0
     f54:	eccff0ef          	jal	620 <printf_core>
}
     f58:	0a813083          	ld	ra,168(sp)
     f5c:	0a013403          	ld	s0,160(sp)
     f60:	09813483          	ld	s1,152(sp)
     f64:	0b010113          	addi	sp,sp,176
     f68:	00008067          	ret

0000000000000f6c <__udivsi3>:
# define __divdi3 __divsi3
# define __moddi3 __modsi3
#else
FUNC_BEGIN (__udivsi3)
  /* Compute __udivdi3(a0 << 32, a1 << 32); cast result to uint32_t.  */
  sll    a0, a0, 32
     f6c:	02051513          	slli	a0,a0,0x20
  sll    a1, a1, 32
     f70:	02059593          	slli	a1,a1,0x20
  move   t0, ra
     f74:	00008293          	mv	t0,ra
  jal    HIDDEN_JUMPTARGET(__udivdi3)
     f78:	03c000ef          	jal	fb4 <__hidden___udivdi3>
  sext.w a0, a0
     f7c:	0005051b          	sext.w	a0,a0
  jr     t0
     f80:	00028067          	jr	t0

0000000000000f84 <__umodsi3>:
FUNC_END (__udivsi3)

FUNC_BEGIN (__umodsi3)
  /* Compute __udivdi3((uint32_t)a0, (uint32_t)a1); cast a1 to uint32_t.  */
  sll    a0, a0, 32
     f84:	02051513          	slli	a0,a0,0x20
  sll    a1, a1, 32
     f88:	02059593          	slli	a1,a1,0x20
  srl    a0, a0, 32
     f8c:	02055513          	srli	a0,a0,0x20
  srl    a1, a1, 32
     f90:	0205d593          	srli	a1,a1,0x20
  move   t0, ra
     f94:	00008293          	mv	t0,ra
  jal    HIDDEN_JUMPTARGET(__udivdi3)
     f98:	01c000ef          	jal	fb4 <__hidden___udivdi3>
  sext.w a0, a1
     f9c:	0005851b          	sext.w	a0,a1
  jr     t0
     fa0:	00028067          	jr	t0

0000000000000fa4 <__divsi3>:

FUNC_ALIAS (__modsi3, __moddi3)

FUNC_BEGIN( __divsi3)
  /* Check for special case of INT_MIN/-1. Otherwise, fall into __divdi3.  */
  li    t0, -1
     fa4:	fff00293          	li	t0,-1
  beq   a1, t0, .L20
     fa8:	0a558c63          	beq	a1,t0,1060 <__moddi3+0x30>

0000000000000fac <__divdi3>:
#endif

FUNC_BEGIN (__divdi3)
  bltz  a0, .L10
     fac:	06054063          	bltz	a0,100c <__umoddi3+0x10>
  bltz  a1, .L11
     fb0:	0605c663          	bltz	a1,101c <__umoddi3+0x20>

0000000000000fb4 <__hidden___udivdi3>:
  /* Since the quotient is positive, fall into __udivdi3.  */

FUNC_BEGIN (__udivdi3)
  mv    a2, a1
     fb4:	00058613          	mv	a2,a1
  mv    a1, a0
     fb8:	00050593          	mv	a1,a0
  li    a0, -1
     fbc:	fff00513          	li	a0,-1
  beqz  a2, .L5
     fc0:	02060c63          	beqz	a2,ff8 <__hidden___udivdi3+0x44>
  li    a3, 1
     fc4:	00100693          	li	a3,1
  bgeu  a2, a1, .L2
     fc8:	00b67a63          	bgeu	a2,a1,fdc <__hidden___udivdi3+0x28>
.L1:
  blez  a2, .L2
     fcc:	00c05863          	blez	a2,fdc <__hidden___udivdi3+0x28>
  slli  a2, a2, 1
     fd0:	00161613          	slli	a2,a2,0x1
  slli  a3, a3, 1
     fd4:	00169693          	slli	a3,a3,0x1
  bgtu  a1, a2, .L1
     fd8:	feb66ae3          	bltu	a2,a1,fcc <__hidden___udivdi3+0x18>
.L2:
  li    a0, 0
     fdc:	00000513          	li	a0,0
.L3:
  bltu  a1, a2, .L4
     fe0:	00c5e663          	bltu	a1,a2,fec <__hidden___udivdi3+0x38>
  sub   a1, a1, a2
     fe4:	40c585b3          	sub	a1,a1,a2
  or    a0, a0, a3
     fe8:	00d56533          	or	a0,a0,a3
.L4:
  srli  a3, a3, 1
     fec:	0016d693          	srli	a3,a3,0x1
  srli  a2, a2, 1
     ff0:	00165613          	srli	a2,a2,0x1
  bnez  a3, .L3
     ff4:	fe0696e3          	bnez	a3,fe0 <__hidden___udivdi3+0x2c>
.L5:
  ret
     ff8:	00008067          	ret

0000000000000ffc <__umoddi3>:
FUNC_END (__udivdi3)
HIDDEN_DEF (__udivdi3)

FUNC_BEGIN (__umoddi3)
  /* Call __udivdi3(a0, a1), then return the remainder, which is in a1.  */
  move  t0, ra
     ffc:	00008293          	mv	t0,ra
  jal   HIDDEN_JUMPTARGET(__udivdi3)
    1000:	fb5ff0ef          	jal	fb4 <__hidden___udivdi3>
  move  a0, a1
    1004:	00058513          	mv	a0,a1
  jr    t0
    1008:	00028067          	jr	t0
FUNC_END (__umoddi3)

  /* Handle negative arguments to __divdi3.  */
.L10:
  neg   a0, a0
    100c:	40a00533          	neg	a0,a0
  /* Zero is handled as a negative so that the result will not be inverted.  */
  bgtz  a1, .L12     /* Compute __udivdi3(-a0, a1), then negate the result.  */
    1010:	00b04863          	bgtz	a1,1020 <__umoddi3+0x24>

  neg   a1, a1
    1014:	40b005b3          	neg	a1,a1
  j     HIDDEN_JUMPTARGET(__udivdi3)     /* Compute __udivdi3(-a0, -a1).  */
    1018:	f9dff06f          	j	fb4 <__hidden___udivdi3>
.L11:                /* Compute __udivdi3(a0, -a1), then negate the result.  */
  neg   a1, a1
    101c:	40b005b3          	neg	a1,a1
.L12:
  move  t0, ra
    1020:	00008293          	mv	t0,ra
  jal   HIDDEN_JUMPTARGET(__udivdi3)
    1024:	f91ff0ef          	jal	fb4 <__hidden___udivdi3>
  neg   a0, a0
    1028:	40a00533          	neg	a0,a0
  jr    t0
    102c:	00028067          	jr	t0

0000000000001030 <__moddi3>:
FUNC_END (__divdi3)

FUNC_BEGIN (__moddi3)
  move   t0, ra
    1030:	00008293          	mv	t0,ra
  bltz   a1, .L31
    1034:	0005ca63          	bltz	a1,1048 <__moddi3+0x18>
  bltz   a0, .L32
    1038:	00054c63          	bltz	a0,1050 <__moddi3+0x20>
.L30:
  jal    HIDDEN_JUMPTARGET(__udivdi3)    /* The dividend is not negative.  */
    103c:	f79ff0ef          	jal	fb4 <__hidden___udivdi3>
  move   a0, a1
    1040:	00058513          	mv	a0,a1
  jr     t0
    1044:	00028067          	jr	t0
.L31:
  neg    a1, a1
    1048:	40b005b3          	neg	a1,a1
  bgez   a0, .L30
    104c:	fe0558e3          	bgez	a0,103c <__moddi3+0xc>
.L32:
  neg    a0, a0
    1050:	40a00533          	neg	a0,a0
  jal    HIDDEN_JUMPTARGET(__udivdi3)    /* The dividend is hella negative.  */
    1054:	f61ff0ef          	jal	fb4 <__hidden___udivdi3>
  neg    a0, a1
    1058:	40b00533          	neg	a0,a1
  jr     t0
    105c:	00028067          	jr	t0
FUNC_END (__moddi3)

#if __riscv_xlen == 64
  /* continuation of __divsi3 */
.L20:
  sll   t0, t0, 31
    1060:	01f29293          	slli	t0,t0,0x1f
  bne   a0, t0, __divdi3
    1064:	f45514e3          	bne	a0,t0,fac <__divdi3>
  ret
    1068:	00008067          	ret

000000000000106c <__muldi3>:
/* Our RV64 64-bit routine is equivalent to our RV32 32-bit routine.  */
# define __muldi3 __mulsi3
#endif

FUNC_BEGIN (__muldi3)
  mv     a2, a0
    106c:	00050613          	mv	a2,a0
  li     a0, 0
    1070:	00000513          	li	a0,0
.L1:
  andi   a3, a1, 1
    1074:	0015f693          	andi	a3,a1,1
  beqz   a3, .L2
    1078:	00068463          	beqz	a3,1080 <__muldi3+0x14>
  add    a0, a0, a2
    107c:	00c50533          	add	a0,a0,a2
.L2:
  srli   a1, a1, 1
    1080:	0015d593          	srli	a1,a1,0x1
  slli   a2, a2, 1
    1084:	00161613          	slli	a2,a2,0x1
  bnez   a1, .L1
    1088:	fe0596e3          	bnez	a1,1074 <__muldi3+0x8>
  ret
    108c:	00008067          	ret
