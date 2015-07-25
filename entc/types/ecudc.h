/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "../system/macros.h"
#include "../system/types.h"
#include "../system/ecfile.h"

#include "ecstring.h"
#include "eccursor.h"

// structure types

#define ENTC_UDC_NODE        0x0000
#define ENTC_UDC_LIST        0x0001

struct EcUdc_s; typedef struct EcUdc_s* EcUdc;

// entc types

#define ENTC_UDC_STRING      0x1000
#define ENTC_UDC_BYTE        0x1001
#define ENTC_UDC_UINT32      0x1002
#define ENTC_UDC_UINT64      0x1003
#define ENTC_UDC_REF         0x1004
#define ENTC_UDC_TIME        0x1005
#define ENTC_UDC_CURSOR      0x1006
#define ENTC_UDC_FILEINFO    0x1007
#define ENTC_UDC_TABLEINFO   0x1008
#define ENTC_UDC_SET         0x1009

typedef struct
{
  
  EcString name;
  
  uint64_t size;
  
} EcTableInfo_s; typedef EcTableInfo_s* EcTableInfo;

typedef struct
{
  
  uint64_t setid;
  
  EcUdc content;
  
} EcSet_s; typedef EcSet_s* EcSet;

// complex types (structs)

#define ENTC_UDC_FILE        0x2000
#define ENTC_UDC_FOLDER      0x2001
#define ENTC_UDC_USERINFO    0x2003

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcUdc ecudc_create (uint_t type, const EcString name);

__LIB_EXPORT void ecudc_destroy (EcUdc*);

__LIB_EXPORT void ecudc_add (EcUdc, EcUdc*);

__LIB_EXPORT int ecudc_del (EcUdc, const EcString name);

__LIB_EXPORT EcUdc ecudc_node (EcUdc, const EcString name);

__LIB_EXPORT EcUdc ecudc_node_e (EcUdc, const EcString name);

__LIB_EXPORT const EcString ecudc_name (EcUdc);

__LIB_EXPORT void ecudc_setName (EcUdc, const EcString);

__LIB_EXPORT uint_t ecudc_type (EcUdc);

__LIB_EXPORT EcUdc ecudc_next (EcUdc, void** cursor);

__LIB_EXPORT EcUdc ecudc_extract (EcUdc, void** cursor);

__LIB_EXPORT void ecudc_protect (EcUdc, ubyte_t mode);

// getter setter

__LIB_EXPORT void ecudc_setS (EcUdc, const EcString value);

__LIB_EXPORT void ecudc_setS_o (EcUdc, EcString*);

__LIB_EXPORT void ecudc_setP (EcUdc, void*);

__LIB_EXPORT void ecudc_setB (EcUdc, ubyte_t);

__LIB_EXPORT void ecudc_setUInt32 (EcUdc, uint32_t);

__LIB_EXPORT void ecudc_setUInt64 (EcUdc, uint64_t);

__LIB_EXPORT void ecudc_setTime (EcUdc, const time_t*);

__LIB_EXPORT const EcString ecudc_asString (EcUdc);

__LIB_EXPORT void* ecudc_asP (EcUdc);

__LIB_EXPORT ubyte_t ecudc_asB (EcUdc);

__LIB_EXPORT uint32_t ecudc_asUInt32 (EcUdc);

__LIB_EXPORT uint64_t ecudc_asUInt64 (EcUdc);

__LIB_EXPORT const time_t* ecudc_asTime (EcUdc);

__LIB_EXPORT EcCursor ecudc_asCursor (EcUdc);

__LIB_EXPORT EcFileInfo ecudc_asFileInfo (EcUdc);

__LIB_EXPORT EcTableInfo ecudc_asTableInfo (EcUdc);

__LIB_EXPORT EcSet ecudc_asSet (EcUdc);

// helper

__LIB_EXPORT void* ecudc_get_asP (const EcUdc, const EcString name, void* alt);

__LIB_EXPORT const EcString ecudc_get_asString (const EcUdc, const EcString name, const EcString alt);

__LIB_EXPORT ubyte_t ecudc_get_asB (const EcUdc, const EcString name, ubyte_t alt);

__LIB_EXPORT uint32_t ecudc_get_asUInt32 (const EcUdc, const EcString name, uint32_t alt);

__LIB_EXPORT uint64_t ecudc_get_asUInt64 (const EcUdc, const EcString name, uint64_t alt);

__LIB_EXPORT const time_t* ecudc_get_asTime (const EcUdc, const EcString name, const time_t* alt);

__LIB_EXPORT void ecudc_add_asP (EcUdc, const EcString name, void* value);

__LIB_EXPORT void ecudc_add_asString (EcUdc, const EcString name, const EcString value);

__LIB_EXPORT void ecudc_add_asS_o (EcUdc, const EcString name, EcString*);

__LIB_EXPORT void ecudc_add_asB (EcUdc, const EcString name, ubyte_t value);

__LIB_EXPORT void ecudc_add_asUInt32 (EcUdc, const EcString name, uint32_t value);

__LIB_EXPORT void ecudc_add_asUInt64 (EcUdc, const EcString name, uint64_t value);

__LIB_EXPORT void ecudc_add_asTime (EcUdc, const EcString name, const time_t* value);

// tools

__LIB_EXPORT EcUdc ecudc_errcode (uint_t errcode);

__CPP_EXTERN______________________________________________________________________________END

#endif

