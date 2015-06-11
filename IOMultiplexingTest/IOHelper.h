#ifndef _IOHELPER_H_
#define _IOHELPER_H_

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

int max(int i, int j);
int readn(int fildes, void *buffer, size_t nbyte);
int writen(int fildes, const void *buffer, size_t nbyte);

#endif

