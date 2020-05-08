#include <unicorn/unicorn.h>

//void Svc_GemeiEmu_fopen(uc_engine* uc, int arg0, int arg1, int arg2, int arg3);
//void Svc_GemeiEmu_panic(uc_engine* uc, int arg0, int arg1, int arg2, int arg3);
//void Svc_GemeiEmu_putc(uc_engine* uc, int c, int arg0, int arg1, int arg2);

typedef int (*SvcHandler)(uc_engine* uc, int arg0, int arg1, int arg2, int arg3);

extern SvcHandler svc_handler_table[];
