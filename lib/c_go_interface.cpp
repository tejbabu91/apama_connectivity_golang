#include "GoTransport.h"

using apamax::golang::GoTransport;

#ifdef __cplusplus
extern "C"
{
#endif

    void c_callback(void *ptr, void *buf, int buflen)
    {
        GoTransport *obj = reinterpret_cast<GoTransport *>(ptr);
        obj->towardsHost((char *)buf, buflen);
    }

#ifdef __cplusplus
}
#endif
