#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#undef errno
extern int errno;

char *__env[1] = { 0 };
char **environ = __env;

void _cpu_init_hook() {
}

void _exit(int rc) {
  asm("ecall");
}

int _close(int file) {
  errno = EBADF;
  return -1;
}

int _execve(const char *name, char * const argv[], char * const env[]) {
  errno = ENOMEM;
  return -1;
}

int _fork() {
  errno = EAGAIN;
  return -1;
}

int _fstat(int file, struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}

int _getpid() {
  return 1;
}

int _isatty(int file) {
  return 1;
}

int _kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}

int _link(const char *old, const char *new) {
  errno = EMLINK;
  return -1;
}

off_t _lseek(int file, off_t ptr, int dir) {
  return 0;
}

int _open(const char *name, int flags, int mode) {
  errno = ENOSYS;
  return -1;
}

int _uart_status() {
  // uart status access here...
  register int FR asm("t0");
  asm("li a1, 0x9000000");
  asm("lw t0, 0x18(a1)");
  return FR;
}

int _inbyte() {
  // if uart receive (buffer) empty return 0...
  if ( (_uart_status() & 0x10) != 0)
    return 0; 
  // uart access here...
  register int DR asm("t0");
  asm("li a1, 0x9000000");
  asm("lw t0, 0(a1)");
  return DR;
}


void * _sbrk(ptrdiff_t incr) {
  errno = ENOMEM;
  return (void *) -1;
}

int _stat(const char *file, struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}

clock_t _times(void *buf) {
  errno = EACCES;
  return -1;
}

int _unlink(const char *name) {
  errno = ENOENT;
  return -1;
}

int _wait(int *status) {
  errno = ECHILD;
  return -1;
}

void _outbyte(char xc) {
  // uart access here...
  asm("li a1, 0x9000000");
  asm("sw a0, 0(a1)");
}

int _write(int file, const void *ptr, size_t len) {
  const char *tbuf = (const char *) ptr;
  int i;
  for (i = 0; i < len; i++) {
     _outbyte(tbuf[i]);
  }
  return len;
}

int _read(int file, void *ptr, size_t len) {
  char *tbuf = (char *) ptr;
  int i;
  for (i = 0; i < len; i++) {
     int nc = _inbyte();
     if (nc == -1)
       break;
     tbuf[i] = (char) nc;
  }
  return i;
}

int _gettimeofday(struct timeval *p, void *z) {
  errno = ENOSYS;
  return -1;
}
