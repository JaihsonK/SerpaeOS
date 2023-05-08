#ifndef STATUS_H
#define STATUS_H

#define SERPAEOS_ALL_OK 0
#define EIO 1
#define EINVARG 2
#define ENOMEM 3
#define EBADPATH 4
#define ENOTUS 5
#define ERDONLY 6
#define EUNIMP 7
#define EISTKN 8
#define EINFORMAT 9
#define ENODISKSPACE 10
#define EINTERNERR 11 
#define ECANNOTDO 12
#define ETIMEDOUT 13
#define ENOTPRIV 14
#define ENOTREADY 15
#define ENORESOURCES 16

#define SERPAEOS_REMOTE_DISCONNECTED 0xDDCECED

#ifndef ERROR
#define ERROR(value) (void*)(value)
#endif

#ifndef ERROR_I
#define ERROR_I(value) (int)(value)
#endif

#ifndef ISERR
#define ISERR(value) ((int)value < 0)
#endif

#endif