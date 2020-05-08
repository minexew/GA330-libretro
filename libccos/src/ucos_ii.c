#include <libccos/ccos_host.h>
#include <libccos/ucos_ii.h>

//OS_EVENT* OSSemCreate(uint16_t cnt) {
//    ccos_trace_stub(OSSemCreate);
//    return (OS_EVENT);
//}

void OSSemPend(OS_EVENT * pevent, INT16U timeout, INT8U * err) {
    ccos_trace_stub(OSSemPend);
}

INT8U OSSemPost(OS_EVENT *pevent) {
    ccos_trace_stub(OSSemPost);
    return 0;
}
