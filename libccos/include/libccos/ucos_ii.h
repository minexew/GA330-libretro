#ifndef LIBCCOS_UCOS_II_H
#define LIBCCOS_UCOS_II_H

#include <stdint.h>

typedef struct OS_EVENT OS_EVENT;
typedef uint8_t INT8U;
typedef uint16_t INT16U;

//OS_EVENT *OSSemCreate (INT16U cnt);
void OSSemPend(OS_EVENT * pevent, INT16U timeout, INT8U * err);
INT8U  OSSemPost (OS_EVENT *pevent);

#endif
