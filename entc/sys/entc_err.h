#ifndef __ENTC_SYS__ERR__H
#define __ENTC_SYS__ERR__H 1

#include "sys/entc_export.h"

//=============================================================================

#define ENTC_ERR_NONE           0x0000
#define ENTC_ERR_OS             0x0001
#define ENTC_ERR_LIB            0x0002
#define ENTC_ERR_3RDPARTY_LIB   0x0004
#define ENTC_ERR_NO_OBJECT      0x0005
#define ENTC_ERR_RUNTIME        0x0006
#define ENTC_ERR_CONTINUE       0x0007
#define ENTC_ERR_PARSER         0x0008
#define ENTC_ERR_NOT_FOUND      0x0009
#define ENTC_ERR_MISSING_PARAM  0x000A

//=============================================================================

struct EntcErr_s; typedef struct EntcErr_s* EntcErr;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcErr           entc_err_new           (void);            // allocate memory and initialize the object

__ENTC_LIBEX   void              entc_err_del           (EntcErr*);         // release memory

//-----------------------------------------------------------------------------

__ENTC_LIBEX   int               entc_err_set           (EntcErr, unsigned long errCode, const char* error_message);

__ENTC_LIBEX   int               entc_err_set_fmt       (EntcErr, unsigned long errCode, const char* error_message, ...);

__ENTC_LIBEX   const char*       entc_err_text          (EntcErr);

__ENTC_LIBEX   unsigned long     entc_err_code          (EntcErr);

__ENTC_LIBEX   int               entc_err_lastOSError   (EntcErr);

__ENTC_LIBEX   int               entc_err_formatErrorOS (EntcErr, unsigned long errCode);

//-----------------------------------------------------------------------------

#endif

