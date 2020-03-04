#ifndef _OPENAPI_PLATFORM_H_
#define _OPENAPI_PLATFORM_H_

// specify a platform
// only ONE of following platform should be defined:

//#define OA_PLT_WINDOWS          1
//#define OA_PLT_LINUX          1
//#define OA_PLT_BSD            1
//#define OA_PLT_ANDROID        1

#if OA_PLT_LINUX || OA_PLT_BSD || OA_PLT_ANDROID || OA_PLT_MACOSX
#define OA_PLT_UNIX_FAMILY      1 
#endif

#if OA_PLT_LINUX != 1 \
    && OA_PLT_BSD != 1 \
    && OA_PLT_ANDROID != 1 \
    && OA_PLT_MACOSX != 1 \
    && OA_PLT_WINDOWS != 1
#error OS Platform macro is not specifed!
#endif

#endif//_OPENAPI_PLATFORM_H_
