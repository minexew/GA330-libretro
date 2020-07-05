#ifndef LIBCCOS_MALLOC_H
#define LIBCCOS_MALLOC_H

#include <stdlib.h>

void* OSMalloc(size_t size);
void OSFree(void* ptr);

#endif
