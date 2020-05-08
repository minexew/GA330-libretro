#!/usr/bin/env python3
# CSV format: name, address (ignored), type of thunk, where defined originally

import csv
from dataclasses import dataclass
from enum import auto, Enum
import sys

class Type(Enum):
    ABORT = auto()
    EMU = auto()
    NOOP = auto()
    MINISYS = auto()
    STUB = auto()
    STUB_DUMMY_ALLOC = auto()

@dataclass
class Entry:
    index: int
    name: str
    c_name: str
    type: Type

IN, MINISYS_C_OUT, MINISYS_H_OUT, NATIVE_OUT = sys.argv[1:]

with open(IN, "rt") as f, open(MINISYS_C_OUT, "wt") as mcf, open(MINISYS_H_OUT, "wt") as mhf, open(NATIVE_OUT, "wt") as nf:
    reader = csv.reader(f)
    name_map = {}

    nf.write("""/* GENERATED FILE, DO NOT EDIT */
#include "svc_handlers.h"

""")
    mcf.write("""/* GENERATED FILE, DO NOT EDIT */
#include "thunks.h"

""")

    mhf.write("""/* GENERATED FILE, DO NOT EDIT */
#include "thunksdef.h"

""")

    next_svc = 0
    all_ = []
    for name, type, origin in reader:
        type = Type[type]
        decl_in_minisys = type != Type.MINISYS
        stub_in_minisys = type in {Type.ABORT, Type.NOOP, Type.STUB, Type.STUB_DUMMY_ALLOC}

        if stub_in_minisys:
            c_name = "stub_" + name
        else:
            c_name = name

        all_.append(Entry(next_svc, name, c_name, type))

        # miniSYS thunk proto
        # if type in {Type.NOOP}:
        #     proto = f"void {c_name}(void);"
        #     mhf.write(proto + "\n")
        # elif type in {Type.stub}:
        if decl_in_minisys:
            minisys_proto = f"int {c_name}(int arg1, int arg2, int arg3, int arg4)"
            mhf.write(minisys_proto + ";\n")
        else:
            minisys_proto = None

        # minisys thunk
        if type in {Type.ABORT}:
            thunk = f"""
{minisys_proto} {{
    ccos_panic("Unimplemented call {name}");
}}
"""
            mcf.write(thunk)
        if type in {Type.EMU}:
            thunk = f"""
__attribute__((naked)) {minisys_proto} {{
    __asm(
        "svc {next_svc} \\r\\n"
        "bx lr \\r\\n"
        );
}}
"""
            mcf.write(thunk)
        elif type in {Type.STUB}:
            thunk = f"""
{minisys_proto} {{
    ccos_trace_stub({name});
    return 0xCCCCCCCC;
}}
"""
            mcf.write(thunk)
        elif type in {Type.STUB_DUMMY_ALLOC}:
            thunk = f"""
{minisys_proto} {{
    ccos_trace_stub({name});
    return (int) malloc(1);
}}
"""
            mcf.write(thunk)
        elif type in {Type.NOOP}:
            thunk = f"""
{minisys_proto} {{
    return 0xCCCCCCCC;
}}
"""
            mcf.write(thunk)

        if type in {Type.EMU}:
            # generate native prototype
            proto = f"""
int Svc_{name}(uc_engine* uc, int arg0, int arg1, int arg2, int arg3);
"""
            nf.write(proto)
#         elif type in {Type.stub}:
#             # generate native handler
#             thunk = f"""
# __attribute__((weak)) int Svc_{name}(uc_engine* uc, int arg0, int arg1, int arg2, int arg3) {{
#     printf("THUNK {name} (%08X, %08X, %08X, %08X)\\n", arg0, arg1, arg2, arg3);
#     abort();
# }}
# """
#             nf.write(thunk)

        if type == Type.EMU:
            next_svc += 1

    # generate native handler table

    nf.write("SvcHandler svc_handler_table[] = {\n")
    for e in all_:
        if e.type == Type.EMU:
            nf.write(f"    &Svc_{e.name},\n")
    nf.write("};\n")

    mhf.write("\n")
    mhf.write("extern OsFunction os_function_table[];\n")
    mcf.write("OsFunction os_function_table[] = {\n")
    for e in all_:
        mcf.write(f"""    {{"{e.name}", (int (*)(int,  int,  int,  int)) &{e.c_name}}},\n""")
    mcf.write("    {0, 0},\n")
    mcf.write("};\n")
