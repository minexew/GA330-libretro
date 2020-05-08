#!/usr/bin/env python3
# CSV format: name, address (ignored), type of thunk, where defined originally

import csv
import sys

IN, MINISYS_C_OUT, MINISYS_H_OUT, NATIVE_OUT = sys.argv[1:]

with open(IN, "rt") as f, open(MINISYS_C_OUT, "wt") as mcf, open(MINISYS_H_OUT, "wt") as mhf, open(NATIVE_OUT, "wt") as nf:
    reader = csv.reader(f)

    nf.write("""/* GENERATED FILE, DO NOT EDIT */
#include "svc_handlers.h"

""")
    mcf.write("""/* GENERATED FILE, DO NOT EDIT */
#include "thunks.h"

""")

    mhf.write("""/* GENERATED FILE, DO NOT EDIT */
#include "thunksdef.h"

""")

    i = 0
    all_ = []
    for name, addr, type, origin in reader:
        all_.append((i, name, int(addr, 16), type))

        if type in {"f"}:
            proto = f"int {name}(int arg1, int arg2, int arg3, int arg4);"
            mhf.write(proto + "\n")

        if type in {"f", "emu"}:
            # generate thunk for minisys

            thunk = f"""
__attribute__((naked)) int {name}(int arg1, int arg2, int arg3, int arg4) {{
    __asm(
        "svc {i} \\r\\n"
        "bx lr \\r\\n"
        );
}}
"""
            mcf.write(thunk)

        if type in {"f"}:
            # generate native handler
            thunk = f"""
__attribute__((weak)) int Svc_{name}(uc_engine* uc, int arg0, int arg1, int arg2, int arg3) {{
    printf("THUNK {name} (%08X, %08X, %08X, %08X)\\n", arg0, arg1, arg2, arg3);
    abort();
}}
"""
            nf.write(thunk)

        i += 1

    # generate native handler table

    nf.write("SvcHandler svc_handler_table[] = {\n")
    for i, name, addr, type in all_:
        if type in {"f", "emu"}:
            nf.write(f"    &Svc_{name},\n")
        else:
            nf.write(f"    NULL,\n")
    nf.write("};\n")

    mhf.write("void patch_executable(void);\n")

    mcf.write("void patch_executable(void) {\n")
    for i, name, addr, type in all_:
        if not addr: continue   # dynamic load only

        mcf.write(f"""    // {name}
    //*(unsigned int*)({addr}) = 0xef000000 | {i};
    //*(unsigned int*)({addr + 4}) = 0xe12fff1e;
    *(unsigned int*)({addr}) = 0xe51ff004;      // ldr  pc, [pc, #-4]
    *(unsigned int*)({addr + 4}) = (int) &{name};
""")
    mcf.write("}\n")

    mhf.write("\n")
    mhf.write("extern OsFunction os_function_table[];\n")
    mcf.write("OsFunction os_function_table[] = {\n")
    for i, name, addr, type in all_:
        mcf.write(f"""    {{"{name}", (int (*)(int,  int,  int,  int)) &{name}}},\n""")
    mcf.write("    {0, 0},\n")
    mcf.write("};\n")
