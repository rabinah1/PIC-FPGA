#ifndef MOCK_SYSCALLS_H
#define MOCK_SYSCALLS_H

#include <fcntl.h>

#define O_RDWR 1
#define O_SYNC 2

typedef int (*open_function)(const char *, int, ...);
extern open_function open;
extern int mock_open(const char *file, int opts, ...);

#endif
