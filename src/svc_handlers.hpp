#include <unicorn/unicorn.h>

typedef void (*SvcHandler)(uc_engine* uc);

extern SvcHandler svc_handler_table[];
