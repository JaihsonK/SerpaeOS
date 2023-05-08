#ifndef ERRNO_H
#define ERRNO_H

extern int *_errno_location();

extern const char *strerror(int);

extern void perror(char *str);

#define errno (*(_errno_location()))

enum errnos
{
    SUCCESS,
    EPERM,
    ENOENT,
    ESRCH,
    EINTR,
    EIO,
    ENXIO,
    E2BIG,
    ENOEXEC,
    EBADF,
    ECHILD,
    EAGAIN,
    ENOMEM,
    EACCES
};

#endif