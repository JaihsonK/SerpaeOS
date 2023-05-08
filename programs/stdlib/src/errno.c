extern int printf(const char *, ...);

static int errno = 0;

#define std_errors_count 13

static const char *str_errors [std_errors_count + 1] = {
    "No Error!",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "I/O error",
    "No such device or address",
    "Argument list too long",
    "Exec format error",
    "Bad file number",
    "No child processes",
    "Try Again",
    "Out of memory",
    "Permission denied"
};

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


int *_errno_location()
{
    return &errno;
}

const char *strerror(int errnum)
{
    if(errnum > std_errors_count || errnum < 0) return 0;
    return str_errors[errnum];
}

void perror(char *str)
{
    printf("%s: %s", str, strerror(errno));
}