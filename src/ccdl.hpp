#ifndef GA330_CCDL_H
#define GA330_CCDL_H

#include <cstdint>

int load_rom(const char* path);

void ccdl_run_frame();
void ccdl_get_framebuffer(uint16_t* frame_buf_out);

#endif
