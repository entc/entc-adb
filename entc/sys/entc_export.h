#ifndef __ENTC_SYS__EXPORT__H
#define __ENTC_SYS__EXPORT__H 1

#if defined _WIN64 || defined _WIN32

#define __ENTC_LIBEX  __declspec( dllexport )
#define __STDCALL __stdcall

#else

#ifdef __cplusplus
  #define __ENTC_LIBEX extern "C" 
#else
  #define __ENTC_LIBEX
#endif

#define __STDCALL

#endif

#define TRUE 1
#define FALSE 0

#endif
