#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
//#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>

#include "thunks.h"

//#undef errno
//extern int errno;

extern int __io_getchar(void) __attribute__((weak));

//#define MAX_STACK_SIZE 0x2000

enum { RAM_START = 0x10000000 };
enum { RAM_END = 0x04000000 };

void* _sbrk(int incr) {
    static char* heap_end;
    char* prev_heap_end;

    if (heap_end == 0) {
        heap_end = (char*) RAM_START;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    return (void*) prev_heap_end;
}

/*
 * _gettimeofday primitive (Stub function)
 * */
//int _gettimeofday (struct timeval* tp, struct timezone* tzp) {
//    /* Return fixed data for the timezone.  */
//    if (tzp) {
//        tzp->tz_minuteswest = 0;
//        tzp->tz_dsttime = 0;
//    }
//
//    return 0;
//}
//void initialise_monitor_handles() {
//}

//int _getpid(void) {
//    return 1;
//}
//
//int _kill(int pid, int sig) {
//    errno = EINVAL;
//    return -1;
//}
//
//void _exit (int status) {
//    _kill(status, -1);
//    while (1) {}
//}

int _write(int file, char* ptr, int len) {
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        GemeiEmu_putc( *ptr++, 0, 0, 0 );
    }
    return len;
}

int _close(int file) {
    return -1;
}

int _fstat(int file, struct stat* st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

int _read(int file, char* ptr, int len) {
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        *ptr++ = __io_getchar();
    }

    return len;
}

//int _open(char* path, int flags, ...) {
//    /* Pretend like we always fail */
//    return -1;
//}
//
//int _wait(int* status) {
//    errno = ECHILD;
//    return -1;
//}
//
//int _unlink(char* name) {
//    errno = ENOENT;
//    return -1;
//}
//
//int _times(struct tms* buf) {
//    return -1;
//}
//
//int _stat(char* file, struct stat* st) {
//    st->st_mode = S_IFCHR;
//    return 0;
//}
//
//int _link(char* old, char* new) {
//    errno = EMLINK;
//    return -1;
//}
//
//int _fork(void) {
//    errno = EAGAIN;
//    return -1;
//}
//
//int _execve(char* name, char** argv, char** env) {
//    errno = ENOMEM;
//    return -1;
//}
