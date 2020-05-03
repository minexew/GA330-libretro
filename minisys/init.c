#include "thunks.h"

enum { LOAD_ADDRESS = 0x10100000 };
enum { ENTRY_ADDRESS = 0x10100150 };
enum { APPMAIN_ADDRESS = 0x101000B4 };

// Ensure that the first instruction of the binary is a jump to _start
__asm(
    "b _start \r\n"
    "ldr pc, =0xAABBCCDD \r\n"
);

void _start(void) {
    printf("miniSYS _start\n", 0, 0, 0);
    patch_executable();

    ((int (*)(int, int))ENTRY_ADDRESS)(0, 0);
    ((int (*)(int))APPMAIN_ADDRESS)(LOAD_ADDRESS);
}

// stolen somewhere
static int _strcmp(char const* p1, char const* p2) {
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;
    do
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}

int dl_get_proc(int arg1, int arg2, int arg3, int arg4) {
    char* name = arg2;

    printf((int)name, 0, 0, 0);
    for (int i = 0; os_function_table[i].name; i++) {
        if (!_strcmp(os_function_table[i].name, name)) {
            return (int) os_function_table[i].addr;
        }
    }
    return 0xcccccccc;
}
