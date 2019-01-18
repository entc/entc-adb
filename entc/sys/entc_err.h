/*
 Copyright (c) 2019 Alexander Kalkhof
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#ifndef __ENTC_SYS__ERR__H
#define __ENTC_SYS__ERR__H 1

#include "sys/entc_export.h"

//=============================================================================

#define ENTC_ERR_NONE                0
#define ENTC_ERR_NONE_CONTINUE       1

#define ENTC_ERR_NOT_FOUND           2
#define ENTC_ERR_NOT_SUPPORTED       3
#define ENTC_ERR_RUNTIME             4
#define ENTC_ERR_EOF                 5
#define ENTC_ERR_OS                  6

#define ENTC_ERR_LIB                 7
#define ENTC_ERR_3RDPARTY_LIB        8

#define ENTC_ERR_NO_OBJECT           9
#define ENTC_ERR_NO_ROLE            10
#define ENTC_ERR_NO_AUTH            11

#define ENTC_ERR_PARSER             12
#define ENTC_ERR_MISSING_PARAM      13
#define ENTC_ERR_PROCESS_ABORT      14
#define ENTC_ERR_PROCESS_FAILED     15
#define ENTC_ERR_WRONG_STATE        16
#define ENTC_ERR_WRONG_VALUE        17

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

