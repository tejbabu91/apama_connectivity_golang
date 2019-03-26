#include "HTTPClient.h"

// typedef void (*CBType) (void);

// static CBType cb;

using com::softwareag::samples::HTTPClient;

void callback(void* ptr) {
    HTTPClient* obj = reinterpret_cast<HTTPClient*>(ptr);
    obj->towardsHost();
}

#ifdef __cplusplus
extern "C" {
#endif

void c_callback(void *ptr) {
    callback(ptr);
}

#ifdef __cplusplus
}
#endif
