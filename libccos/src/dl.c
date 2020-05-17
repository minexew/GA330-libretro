#include <libccos/ccos_host.h>
#include <libccos/dl.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//#define TRACE(args) printf args
#define TRACE(args) do {} while(0)

enum {
    GENERAL_ERROR = -1,
};

// FIXME: assert 32-bit little endian ARM
int ccdl_from_memory(DL_t* dl_out, uint8_t const* file_bytes, size_t file_size) {
    TRACE(("ccdl_from_memory(%p, %zd)\n", file_bytes, file_size));

    CCDL_header_t hdr;
    memcpy(&hdr, file_bytes, sizeof(hdr));
    TRACE(("sign=%s version=%08X unk_8=%08X num_sections=%d\n", hdr.sign, hdr.version, hdr.unk_8, (int) hdr.num_sections));

    if (memcmp(hdr.sign, "CCDL", 4) != 0) {
        ccos_diag("Not a CCDL file");
        return GENERAL_ERROR;
    }

    if (hdr.version != 0x00010000 || hdr.unk_8 != 0x00020001) {
        ccos_diag("Invalid file version");
        return GENERAL_ERROR;
    }

    // load image
    bool have_code = false;
    CCDL_section_header_t shdr_code;

    for (size_t i = 0; i < hdr.num_sections; i++) {
        memcpy(&shdr_code, file_bytes + sizeof(CCDL_header_t) + i * sizeof(CCDL_section_header_t), sizeof(CCDL_section_header_t));

        TRACE(("SHDR[%zd] type=%d\n", i, shdr_code.type));

        if (shdr_code.type != CCDL_SECTION_CODE) {
            continue;
        }

        memcpy((void*) shdr_code.specific_data.code.load_address, file_bytes + shdr_code.offset, shdr_code.size);

        // we keep the section header, as it will be useful later
        have_code = true;
        break;
    }

    if (!have_code) {
        ccos_diag("No RAWD section in CCDL");
        return GENERAL_ERROR;
    }

    // resolve imports
    for (size_t i = 0; i < hdr.num_sections; i++) {
        CCDL_section_header_t shdr;

        memcpy(&shdr, file_bytes + sizeof(CCDL_header_t) + i * sizeof(CCDL_section_header_t), sizeof(CCDL_section_header_t));

        TRACE(("SHDR[%zd] type=%d\n", i, shdr.type));

        if (shdr.type != CCDL_SECTION_IMPORT) {
            continue;
        }

        uint8_t const* p = file_bytes + shdr.offset;

        CCDL_import_export_header_t iehdr;
        memcpy(&iehdr, p, sizeof(iehdr));
        p += sizeof(iehdr);

        char const* string_table = (char const*) p + iehdr.num_entries * sizeof(CCDL_import_export_entry_t);

        TRACE(("%d imports\n", iehdr.num_entries));

        for (uint32_t i = 0; i < iehdr.num_entries; i++) {
            CCDL_import_export_entry_t ent;
            memcpy(&ent, p, sizeof(ent));
            p += sizeof(ent);

            char const* name = string_table + ent.name_offset;
            TRACE((" : '%s' @ %08X\n", name, ent.address));

            uint32_t thunk[2];
            thunk[0] = 0xe51ff004;      // ldr  pc, [pc, #-4]
            thunk[1] = (uint32_t) ccos_get_builtin_proc(name);
            memcpy((void*) ent.address, thunk, sizeof(thunk));
        }

        break;
    }

    // call entry point

    TRACE(("calling entry point\n"));
    ((int (*)(int, int)) shdr_code.specific_data.code.entry_point)(0, 0);
    TRACE(("returned from entry point\n"));

    dl_out->file_bytes = file_bytes;
    dl_out->file_size = file_size;

    return 0;
}

void* dl_get_proc(DL_t* dl, char const* name) {
    TRACE(("dl_get_proc(%p, %s)\n", dl, name));

    CCDL_header_t hdr;
    memcpy(&hdr, dl->file_bytes, sizeof(hdr));
    TRACE(("sign=%s version=%08X unk_8=%08X num_sections=%d\n", hdr.sign, hdr.version, hdr.unk_8, (int) hdr.num_sections));

    // look for exported function
    for (size_t i = 0; i < hdr.num_sections; i++) {
        CCDL_section_header_t shdr;

        memcpy(&shdr, dl->file_bytes + sizeof(CCDL_header_t) + i * sizeof(CCDL_section_header_t), sizeof(CCDL_section_header_t));

        TRACE(("SHDR[%zd] type=%d\n", i, shdr.type));

        if (shdr.type != CCDL_SECTION_EXPORT) {
            continue;
        }

        uint8_t const* p = dl->file_bytes + shdr.offset;

        CCDL_import_export_header_t iehdr;
        memcpy(&iehdr, p, sizeof(iehdr));
        p += sizeof(iehdr);

        char const* string_table = (char const*) p + iehdr.num_entries * sizeof(CCDL_import_export_entry_t);

        TRACE(("%d exports\n", iehdr.num_entries));

        for (uint32_t i = 0; i < iehdr.num_entries; i++) {
            CCDL_import_export_entry_t ent;
            memcpy(&ent, p, sizeof(ent));
            p += sizeof(ent);

            char const* ent_name = string_table + ent.name_offset;
            TRACE((" : '%s' @ %08X\n", ent_name, ent.address));

            if (strcmp(ent_name, name) == 0) {
                return (void*) ent.address;
            }
        }

        break;
    }

    return ccos_get_builtin_proc(name);
}

void* dl_res_open(void* dl, int unk_4, const char* name) {
    printf("dl_res_open(%p, %d, %s)\n", dl, unk_4, name);
    ccos_panic("unimplemented shit");
}
