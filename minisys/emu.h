#ifndef MINISYS_EMU_H
#define MINISYS_EMU_H

int GemeiEmu_fopen(const char* path, const char* mode);

void GemeiEmu_putc(int c);
void GemeiEmu_panic(void) __attribute__((noreturn));

#endif
