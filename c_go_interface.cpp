#include "GoTransport.h"

// typedef void (*CBType) (void);

// static CBType cb;

using apamax::golang::GoTransport;

// void callback(void* ptr, char *buf, int buflen) {
//     HTTPClient* obj = reinterpret_cast<HTTPClient*>(ptr);
//     obj->towardsHost(buf, buflen);
// }

#ifdef __cplusplus
extern "C" {
#endif

void c_callback(void *ptr, void *buf, int buflen) {
    GoTransport* obj = reinterpret_cast<GoTransport*>(ptr);
    obj->towardsHost((char*)buf, buflen);
}

#ifdef __cplusplus
}
#endif
