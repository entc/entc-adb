#ifndef __ENTC_SYS__EXPORT__H
#define __ENTC_SYS__EXPORT__H 1

//----------------------------------------------------------------------------------
#ifdef __cplusplus
#define __EXTERN_C    extern "C"
#else
#define __EXTERN_C
#endif

//----------------------------------------------------------------------------------
#if defined _WIN64 || defined _WIN32

#define __WIN_OS
#define __ENTC_LIBEX __EXTERN_C __declspec(dllexport)

#define __STDCALL __stdcall

//----------------------------------------------------------------------------------
#elif defined __APPLE__

#define __BSD_OS 1

#define __ENTC_LIBEX __EXTERN_C
#define __STDCALL

//----------------------------------------------------------------------------------
#elif defined __bsdi__ || defined __OpenBSD__ || defined __FreeBSD__ || defined __NetBSD__ || __DragonFly__

#define __BSD_OS 1

#define __ENTC_LIBEX __EXTERN_C
#define __STDCALL

//----------------------------------------------------------------------------------
#elif __linux__

#define __LINUX_OS 1

#define __ENTC_LIBEX __EXTERN_C
#define __STDCALL

//----------------------------------------------------------------------------------
#endif

#endif
