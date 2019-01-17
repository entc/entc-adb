/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TOOLS_JPARSER_H
#define ENTC_TOOLS_JPARSER_H 1

//=============================================================================

#include "sys/entc_export.h"
#include "types/ecstring.h"
#include "types/ecerr.h"

//-----------------------------------------------------------------------------

#define ENTC_JPARSER_UNDEFINED       0
#define ENTC_JPARSER_OBJECT_NODE     1
#define ENTC_JPARSER_OBJECT_LIST     2
#define ENTC_JPARSER_OBJECT_TEXT     3
#define ENTC_JPARSER_OBJECT_NUMBER   4
#define ENTC_JPARSER_OBJECT_FLOAT    5
#define ENTC_JPARSER_OBJECT_BOLEAN   6
#define ENTC_JPARSER_OBJECT_NULL     7

//-----------------------------------------------------------------------------

struct EcJsonParser_s; typedef struct EcJsonParser_s* EcJsonParser;

typedef void   (__STDCALL *fct_ecjparser_onItem) (void* ptr, void* obj, int type, void* val, const char* key, int index);
typedef void*  (__STDCALL *fct_ecjparser_onObjCreate) (void* ptr, int type);
typedef void   (__STDCALL *fct_ecjparser_onObjDestroy) (void* ptr, void* obj);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcJsonParser ecjsonparser_create (fct_ecjparser_onItem, fct_ecjparser_onObjCreate, fct_ecjparser_onObjDestroy, void* ptr);

__ENTC_LIBEX void ecjsonparser_destroy (EcJsonParser* pself);

__ENTC_LIBEX int ecjsonparser_parse (EcJsonParser, const char* buffer, int64_t size, EcErr err);

__ENTC_LIBEX void* ecjsonparser_lastObject (EcJsonParser);

//-----------------------------------------------------------------------------

#endif
