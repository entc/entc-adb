/*
 * Copyright (c) 2010-2016 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TYPES_UDC_H
#define ENTC_TYPES_UDC_H 1

#include "types/ecalloc.h"

#include "system/ecfile.h"

#include "ecstring.h"
#include "ecbuffer.h"
#include "eccursor.h"

// structure types

#define ENTC_UDC_NODE        0x0000
#define ENTC_UDC_LIST        0x0001

struct EcUdc_s; typedef struct EcUdc_s* EcUdc;

// basic types

#define ENTC_UDC_STRING      0x1000
#define ENTC_UDC_BYTE        0x1001
#define ENTC_UDC_UBYTE       0x1010
#define ENTC_UDC_INT16       0x1014
#define ENTC_UDC_UINT16      0x1011
#define ENTC_UDC_INT32       0x1012
#define ENTC_UDC_UINT32      0x1002
#define ENTC_UDC_INT64       0x1013
#define ENTC_UDC_UINT64      0x1003
#define ENTC_UDC_FLOAT       0x0021
#define ENTC_UDC_DOUBLE      0x0020
#define ENTC_UDC_REF         0x1004
#define ENTC_UDC_TIME        0x1005
#define ENTC_UDC_CURSOR      0x1006
#define ENTC_UDC_FILEINFO    0x1007
#define ENTC_UDC_TABLEINFO   0x1008
#define ENTC_UDC_SET         0x1009
#define ENTC_UDC_USERINFO    0x100a
#define ENTC_UDC_ERROR       0x100b
#define ENTC_UDC_METHOD      0x100c
#define ENTC_UDC_BUFFER      0x100d
#define ENTC_UDC_BOOL        0x100e
#define ENTC_UDC_NONE        0x100f

typedef struct
{
  
  EcString name;
  
  uint64_t size;
  
} EcTableInfo_s; typedef EcTableInfo_s* EcTableInfo;

typedef struct
{
  
  EcUdc setid;
  
  EcUdc content;
  
} EcSet_s; typedef EcSet_s* EcSet;

typedef struct
{
  
  EcString name;
  
  uint64_t uid;

  uint32_t acc_type;
  
  EcUdc extras;
  
} EcUserInfo_s; typedef EcUserInfo_s* EcUserInfo;

typedef struct
{
  
  uint32_t code;
  
  EcString text;
  
} EcError_s; typedef EcError_s* EcError;

typedef struct
{
  
  EcString name;
  
  ubyte_t version;
  
  EcUdc error;
  
  EcUdc params;
  
  EcUdc result;
  
} EcMethod_s; typedef EcMethod_s* EcMethod;

// complex types (structs)

#define ENTC_UDC_FILE        0x2000
#define ENTC_UDC_FOLDER      0x2001

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcUdc ecudc_create (EcAlloc alloc, uint_t type, const EcString name);

__LIB_EXPORT void ecudc_destroy (EcAlloc alloc, EcUdc*);

__LIB_EXPORT void ecudc_add (EcUdc, EcUdc*);

__LIB_EXPORT int ecudc_del (EcAlloc alloc, EcUdc, const EcString name);

__LIB_EXPORT EcUdc ecudc_node (EcUdc, const EcString name);

__LIB_EXPORT EcUdc ecudc_node_e (EcUdc, const EcString name);

__LIB_EXPORT const EcString ecudc_name (EcUdc);

__LIB_EXPORT void ecudc_setName (EcUdc, const EcString);

__LIB_EXPORT uint_t ecudc_type (EcUdc);

__LIB_EXPORT EcUdc ecudc_next (EcUdc, void** cursor);

__LIB_EXPORT EcUdc ecudc_cursor_e (EcUdc, void** cursor);

// setter

__LIB_EXPORT void ecudc_refUInt32 (EcUdc, uint32_t** ref);

__LIB_EXPORT void ecudc_setS (EcUdc, const EcString value);

__LIB_EXPORT void ecudc_setS_o (EcUdc, EcString*);

__LIB_EXPORT void ecudc_setP (EcUdc, void*);

__LIB_EXPORT void ecudc_setByte (EcUdc, byte_t);

__LIB_EXPORT void ecudc_setUByte (EcUdc, ubyte_t);

__LIB_EXPORT void ecudc_setInt16 (EcUdc, int16_t);

__LIB_EXPORT void ecudc_setUInt16 (EcUdc, uint16_t);

__LIB_EXPORT void ecudc_setInt32 (EcUdc, int32_t);

__LIB_EXPORT void ecudc_setUInt32 (EcUdc, uint32_t);

__LIB_EXPORT void ecudc_setInt64 (EcUdc, int64_t);

__LIB_EXPORT void ecudc_setUInt64 (EcUdc, uint64_t);

__LIB_EXPORT void ecudc_setFloat (EcUdc, float);

__LIB_EXPORT void ecudc_setDouble (EcUdc, double);

__LIB_EXPORT void ecudc_setTime (EcUdc, const time_t*);

__LIB_EXPORT void ecudc_setB_o (EcUdc, EcBuffer*);

__LIB_EXPORT void ecudc_setBool (EcUdc, int);

// getter

__LIB_EXPORT const EcString ecudc_asString (EcUdc);

__LIB_EXPORT EcString ecudc_getString (EcUdc);

__LIB_EXPORT void* ecudc_asP (EcUdc);

__LIB_EXPORT byte_t ecudc_asByte (EcUdc);

__LIB_EXPORT ubyte_t ecudc_asUByte (EcUdc);

__LIB_EXPORT int16_t ecudc_asInt16 (EcUdc);

__LIB_EXPORT uint16_t ecudc_asUInt16 (EcUdc);

__LIB_EXPORT int32_t ecudc_asInt32 (EcUdc);

__LIB_EXPORT uint32_t ecudc_asUInt32 (EcUdc);

__LIB_EXPORT int64_t ecudc_asInt64 (EcUdc);

__LIB_EXPORT uint64_t ecudc_asUInt64 (EcUdc);

__LIB_EXPORT float ecudc_asFloat (EcUdc);

__LIB_EXPORT double ecudc_asDouble (EcUdc);

__LIB_EXPORT const time_t* ecudc_asTime (EcUdc);

__LIB_EXPORT EcCursor ecudc_asCursor (EcUdc);

__LIB_EXPORT EcFileInfo ecudc_asFileInfo (EcUdc);

__LIB_EXPORT EcTableInfo ecudc_asTableInfo (EcUdc);

__LIB_EXPORT EcSet ecudc_asSet (EcUdc);

__LIB_EXPORT EcUserInfo ecudc_asUserInfo (EcUdc);

__LIB_EXPORT EcError ecudc_asError (EcUdc);

__LIB_EXPORT EcMethod ecudc_asMethod (EcUdc);

__LIB_EXPORT EcBuffer ecudc_asB (EcUdc);

__LIB_EXPORT int ecudc_asBool (EcUdc);

// helper

__LIB_EXPORT void* ecudc_get_asP (const EcUdc, const EcString name, void* alt);

__LIB_EXPORT const EcString ecudc_get_asString (const EcUdc, const EcString name, const EcString alt);

__LIB_EXPORT byte_t ecudc_get_asByte (const EcUdc, const EcString name, byte_t alt);

__LIB_EXPORT ubyte_t ecudc_get_asUByte (const EcUdc, const EcString name, ubyte_t alt);

__LIB_EXPORT int16_t ecudc_get_asInt16 (const EcUdc, const EcString name, int16_t alt);

__LIB_EXPORT uint16_t ecudc_get_asUInt16 (const EcUdc, const EcString name, uint16_t alt);

__LIB_EXPORT int32_t ecudc_get_asInt32 (const EcUdc, const EcString name, int32_t alt);

__LIB_EXPORT uint32_t ecudc_get_asUInt32 (const EcUdc, const EcString name, uint32_t alt);

__LIB_EXPORT int64_t ecudc_get_asInt64 (const EcUdc, const EcString name, int64_t alt);

__LIB_EXPORT uint64_t ecudc_get_asUInt64 (const EcUdc, const EcString name, uint64_t alt);

__LIB_EXPORT float ecudc_get_asFloat (const EcUdc, const EcString name, float alt);

__LIB_EXPORT double ecudc_get_asDouble (const EcUdc, const EcString name, double alt);

__LIB_EXPORT const time_t* ecudc_get_asTime (const EcUdc, const EcString name, const time_t* alt);

__LIB_EXPORT EcBuffer ecudc_get_asB (const EcUdc, const EcString name, const EcBuffer alt);

// helper setter

__LIB_EXPORT void ecudc_add_asP (EcAlloc, EcUdc, const EcString name, void* value);

__LIB_EXPORT void ecudc_add_asString (EcAlloc, EcUdc, const EcString name, const EcString value);

__LIB_EXPORT void ecudc_add_asS_o (EcAlloc, EcUdc, const EcString name, EcString*);

__LIB_EXPORT void ecudc_add_asByte (EcAlloc, EcUdc, const EcString name, byte_t value);

__LIB_EXPORT void ecudc_add_asUByte (EcAlloc, EcUdc, const EcString name, ubyte_t value);

__LIB_EXPORT void ecudc_add_asInt16 (EcAlloc, EcUdc, const EcString name, int16_t value);

__LIB_EXPORT void ecudc_add_asUInt16 (EcAlloc, EcUdc, const EcString name, uint16_t value);

__LIB_EXPORT void ecudc_add_asInt32 (EcAlloc, EcUdc, const EcString name, int32_t value);

__LIB_EXPORT void ecudc_add_asUInt32 (EcAlloc, EcUdc, const EcString name, uint32_t value);

__LIB_EXPORT void ecudc_add_asInt64 (EcAlloc, EcUdc, const EcString name, int64_t value);

__LIB_EXPORT void ecudc_add_asUInt64 (EcAlloc, EcUdc, const EcString name, uint64_t value);

__LIB_EXPORT void ecudc_add_asFloat (EcAlloc, EcUdc, const EcString name, float value);

__LIB_EXPORT void ecudc_add_asDouble (EcAlloc, EcUdc, const EcString name, double value);

__LIB_EXPORT void ecudc_add_asTime (EcAlloc, EcUdc, const EcString name, const time_t* value);

__LIB_EXPORT void ecudc_add_asB_o (EcAlloc, EcUdc, const EcString name, EcBuffer*);

// tools

__LIB_EXPORT EcUdc ecudc_errcode (EcAlloc, uint_t errcode);

__CPP_EXTERN______________________________________________________________________________END

#endif

