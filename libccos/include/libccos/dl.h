#ifndef LIBCCOS_CCDL_H
#define LIBCCOS_CCDL_H

// TODO: assert little endian platform

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    char sign[4];
    uint32_t version;
    uint32_t unk_8;
    uint32_t num_sections;
    uint32_t unk_16[4];
} CCDL_header_t;

enum {
    CCDL_SECTION_CODE =         1,
    CCDL_SECTION_RESOURCES =    7,
    CCDL_SECTION_IMPORT =       8,
    CCDL_SECTION_EXPORT =       9,
};

typedef struct {
    char fourcc[4];
    uint32_t type;
    uint32_t offset;
    uint32_t size;

    union {
        struct {
            uint32_t unk_0;
            uint32_t entry_point;
            uint32_t load_address;
            uint32_t alloc_size;
        } code;
    } specific_data;
} CCDL_section_header_t;

typedef struct {
    uint32_t num_entries;
    uint32_t unused[3];
} CCDL_import_export_header_t;

typedef struct {
    char filename[500];
    uint32_t size;
    uint32_t offset;
} CCDL_ERPT_entry_t;

typedef struct {
    uint32_t name_offset;
    uint32_t unk_4;
    uint32_t unk_8;
    uint32_t address;
} CCDL_import_export_entry_t;

typedef struct {
    uint8_t const* file_bytes;
    size_t file_size;

    CCDL_header_t hdr;
} DL_t;

// Matches even ccpmp.bin
typedef struct {
    int start_file_offset;
    int current_file_offset;
    DL_t* dl;
    CCDL_ERPT_entry_t ent;
} DL_res_t;

// file_bytes is not (necessarily) copied -- must stay valid!
int ccdl_from_memory(DL_t* dl_out, uint8_t const* file_bytes, size_t file_size);

void* dl_get_proc(DL_t* dl, char const* name);

DL_res_t* dl_res_open(DL_t* dl, int unk_4, const char* name);
size_t dl_res_get_size(DL_res_t* res);
size_t dl_res_get_data(DL_res_t* res, void* buffer, int size, int count);

#endif
