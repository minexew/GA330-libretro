#include <libccos/malloc.h>

void* OSMalloc(size_t size) {
    return malloc(size);
}

void OSFree(void* ptr) {
    return free(ptr);
}
