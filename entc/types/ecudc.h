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
#define ENTC_UDC_NUMBER      0x1001
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

#pragma pack(push, 16)
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
#pragma pack(pop)

// complex types (structs)

#define ENTC_UDC_FILE        0x2000
#define ENTC_UDC_FOLDER      0x2001

__ENTC_LIBEX EcUdc ecudc_create (EcAlloc alloc, uint_t type, const EcString name);

__ENTC_LIBEX void ecudc_destroy (EcAlloc alloc, EcUdc*);

__ENTC_LIBEX void ecudc_add (EcUdc, EcUdc*);

__ENTC_LIBEX int ecudc_del (EcAlloc alloc, EcUdc, const EcString name);

__ENTC_LIBEX EcUdc ecudc_node (EcUdc, const EcString name);

__ENTC_LIBEX EcUdc ecudc_node_e (EcUdc, const EcString name);

__ENTC_LIBEX const EcString ecudc_name (EcUdc);

__ENTC_LIBEX void ecudc_setName (EcUdc, const EcString);

__ENTC_LIBEX uint_t ecudc_type (EcUdc);

__ENTC_LIBEX EcUdc ecudc_clone (EcAlloc, const EcUdc orig);

__ENTC_LIBEX EcUdc ecudc_next (EcUdc, void** cursor);

__ENTC_LIBEX EcUdc ecudc_cursor_e (EcUdc, void** cursor);

__ENTC_LIBEX void ecudc_merge (EcUdc* dest, EcUdc* part);

__ENTC_LIBEX uint32_t ecudc_size (EcUdc);

// setter

__ENTC_LIBEX void ecudc_refNumber (EcUdc, int64_t** ref);

__ENTC_LIBEX void ecudc_setS (EcUdc, const EcString value);

__ENTC_LIBEX void ecudc_setS_o (EcUdc, EcString*);

__ENTC_LIBEX void ecudc_setP (EcUdc, void*);

__ENTC_LIBEX void ecudc_setNumber (EcUdc, int64_t);

__ENTC_LIBEX void ecudc_setDouble (EcUdc, double);

__ENTC_LIBEX void ecudc_setTime (EcUdc, const time_t*);

__ENTC_LIBEX void ecudc_setB_o (EcUdc, EcBuffer*);

__ENTC_LIBEX void ecudc_setBool (EcUdc, int);

// getter

__ENTC_LIBEX const EcString ecudc_asString (EcUdc);

__ENTC_LIBEX EcString ecudc_getString (EcUdc);

__ENTC_LIBEX void* ecudc_asP (EcUdc);

__ENTC_LIBEX int64_t ecudc_asNumber (EcUdc);

__ENTC_LIBEX double ecudc_asDouble (EcUdc);

__ENTC_LIBEX const time_t* ecudc_asTime (EcUdc);

__ENTC_LIBEX EcCursor ecudc_asCursor (EcUdc);

__ENTC_LIBEX EcFileInfo ecudc_asFileInfo (EcUdc);

__ENTC_LIBEX EcTableInfo ecudc_asTableInfo (EcUdc);

__ENTC_LIBEX EcSet ecudc_asSet (EcUdc);

__ENTC_LIBEX EcUserInfo ecudc_asUserInfo (EcUdc);

__ENTC_LIBEX EcError ecudc_asError (EcUdc);

__ENTC_LIBEX EcMethod ecudc_asMethod (EcUdc);

__ENTC_LIBEX EcBuffer ecudc_asB (EcUdc);

__ENTC_LIBEX int ecudc_asBool (EcUdc);

// helper

__ENTC_LIBEX void* ecudc_get_asP (const EcUdc, const EcString name, void* alt);

__ENTC_LIBEX const EcString ecudc_get_asString (const EcUdc, const EcString name, const EcString alt);

__ENTC_LIBEX int64_t ecudc_get_asNumber (const EcUdc, const EcString name, int64_t alt);

__ENTC_LIBEX double ecudc_get_asDouble (const EcUdc, const EcString name, double alt);

__ENTC_LIBEX const time_t* ecudc_get_asTime (const EcUdc, const EcString name, const time_t* alt);

__ENTC_LIBEX EcBuffer ecudc_get_asB (const EcUdc, const EcString name, const EcBuffer alt);

__ENTC_LIBEX int ecudc_get_asBool (const EcUdc, const EcString name, int alt);

// helper setter

__ENTC_LIBEX void ecudc_add_asP (EcAlloc, EcUdc, const EcString name, void* value);

__ENTC_LIBEX void ecudc_add_asString (EcAlloc, EcUdc, const EcString name, const EcString value);

__ENTC_LIBEX void ecudc_add_asS_o (EcAlloc, EcUdc, const EcString name, EcString*);

__ENTC_LIBEX void ecudc_add_asNumber (EcAlloc, EcUdc, const EcString name, int64_t value);

__ENTC_LIBEX void ecudc_add_asDouble (EcAlloc, EcUdc, const EcString name, double value);

__ENTC_LIBEX void ecudc_add_asTime (EcAlloc, EcUdc, const EcString name, const time_t* value);

__ENTC_LIBEX void ecudc_add_asB_o (EcAlloc, EcUdc, const EcString name, EcBuffer*);

// tools

__ENTC_LIBEX EcUdc ecudc_errcode (EcAlloc, uint_t errcode);

__ENTC_LIBEX EcUdc ecudc_readFromFile (const EcString);

__ENTC_LIBEX int ecudc_writeToFile (const EcString, const EcUdc);

#endif

