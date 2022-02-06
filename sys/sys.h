#ifndef MEM_H
#define MEM_H
#include <stddef.h>

void* syscall(
    void* syscall_number,
    void* param1,
    void* param2,
    void* param3,
    void* param4,
    void* param5
);

typedef unsigned long int uintptr; /* size_t */
typedef long int intptr; /* ssize_t */

static
intptr write(int fd, void const* data, uintptr nbytes)
{
    return (intptr)
        syscall(
            (void*)1, /* SYS_write */
            (void*)(intptr)fd,
            (void*)data,
            (void*)nbytes,
            0, /* ignored */
            0  /* ignored */
        );
}

static
intptr _mmap(void *addr, size_t length, int prot, int flags,
                  int fd)
{
    return (intptr)
        syscall(
            (void*)9, /* SYS_mmap */
            (void*)addr,
            (void*)(size_t)length,
            (void*)prot,
            (void*)flags,
            (void*)fd );
}

#endif