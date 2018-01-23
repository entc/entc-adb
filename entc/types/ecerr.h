/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:alex@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENTC_TYPES_ERR_H
#define ENTC_TYPES_ERR_H 1

#include "system/ecdefs.h"

//=============================================================================

#define ENTC_LVL_IGNORE             3
#define ENTC_LVL_EXPECTED           4
#define ENTC_LVL_ERROR              5
#define ENTC_LVL_FATAL              6

#define ENTC_ERR_NONE               0
#define ENTC_ERR_NONE_CONTINUE      1
#define ENTC_ERR_OS_ERROR           2
#define ENTC_ERR_PROCESS_ABORT      3
#define ENTC_ERR_PARSER             4
#define ENTC_ERR_WRONG_STATE        5
#define ENTC_ERR_WRONG_VALUE        6
#define ENTC_ERR_NOT_FOUND          7
#define ENTC_ERR_EOF                8
#define ENTC_ERR_MISSING_PARAM      9
#define ENTC_ERR_PROCESS_FAILED    10
#define ENTC_ERR_NO_AUTH           11
#define ENTC_ERR_NOT_SUPPORTED     12
#define ENTC_ERR_NO_ROLE           13

//=============================================================================

#pragma pack(push, 16)
struct EcErr_s
{
  int level;
  
  int code;
  
  char* text;
  
};
#pragma pack(pop)

//-----------------------------------------------------------------------------

typedef struct EcErr_s* EcErr;

__LIBEX EcErr ecerr_create ();

__LIBEX void ecerr_destroy (EcErr*);

__LIBEX int ecerr_set (EcErr, int lvl, int code, const char* text);

__LIBEX int ecerr_set_fmt (EcErr, int lvl, int code, const char* text, ...);

__LIBEX int ecerr_setTO (EcErr, int lvl, int code, char** text);

__LIBEX int ecerr_lastErrorOS (EcErr, int lvl);

__LIBEX int ecerr_formatErrorOS (EcErr, int lvl, unsigned long errCode);

//-----------------------------------------------------------------------------

#endif
