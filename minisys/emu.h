#ifndef MINISYS_EMU_H
#define MINISYS_EMU_H

#include <stdint.h>

typedef uint16_t char16_t;

int GemeiEmu_fopen(const char* path, const char* mode);
int GemeiEmu_fopenW(const char16_t* path, const char* mode);

int GemeiEmu_getKeyPad(void);
int GemeiEmu_getTimeMs(void);
void GemeiEmu_sleepMs(int ms);

void GemeiEmu_newFrame(void);

void GemeiEmu_putc(int c);
void GemeiEmu_panic(void) __attribute__((noreturn));

#endif
