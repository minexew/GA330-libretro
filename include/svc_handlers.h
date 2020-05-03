#include <unicorn/unicorn.h>

typedef int (*SvcHandler)(uc_engine* uc, int arg0, int arg1, int arg2, int arg3);

extern SvcHandler svc_handler_table[];
