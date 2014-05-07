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

#include "ecstring.h"

#define ENTC_UDC_NODE    0
#define ENTC_UDC_LIST    1
#define ENTC_UDC_STRING  2
#define ENTC_UDC_BYTE    4
#define ENTC_UDC_LONG    5
#define ENTC_UDC_REF     6

struct EcUdc_s; typedef struct EcUdc_s* EcUdc;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcUdc ecudc_create (uint_t type, const EcString name);

__LIB_EXPORT void ecudc_destroy (EcUdc*);

__LIB_EXPORT void ecudc_add (EcUdc, EcUdc*);

__LIB_EXPORT int ecudc_del (EcUdc, const EcString name);

__LIB_EXPORT EcUdc ecudc_node (EcUdc, const EcString name);

__LIB_EXPORT const EcString ecudc_name (EcUdc);

__LIB_EXPORT uint_t ecudc_type (EcUdc);

__LIB_EXPORT void ecudc_setS (EcUdc, const EcString value);

__LIB_EXPORT void ecudc_setP (EcUdc, void*);

__LIB_EXPORT void ecudc_setB (EcUdc, ubyte_t);

__LIB_EXPORT void ecudc_setL (EcUdc, ulong_t);

__LIB_EXPORT const EcString ecudc_asString (EcUdc);

__LIB_EXPORT void* ecudc_asP (EcUdc);

__LIB_EXPORT ubyte_t ecudc_asB (EcUdc);

__LIB_EXPORT ulong_t ecudc_asL (EcUdc);

__LIB_EXPORT EcUdc ecudc_next (EcUdc, void** cursor);

__LIB_EXPORT void ecudc_protect (EcUdc, ubyte_t mode);

// helper

__LIB_EXPORT void* ecudc_get_asP (const EcUdc, const EcString name, void* alt);

__LIB_EXPORT const EcString ecudc_get_asString (const EcUdc, const EcString name, const EcString alt);

__LIB_EXPORT ubyte_t ecudc_get_asB (const EcUdc, const EcString name, ubyte_t alt);

__LIB_EXPORT ulong_t ecudc_get_asL (const EcUdc, const EcString name, ulong_t alt);

__LIB_EXPORT void ecudc_add_asP (EcUdc, const EcString name, void* value);

__LIB_EXPORT void ecudc_add_asString (EcUdc, const EcString name, const EcString value);

__LIB_EXPORT void ecudc_add_asB (EcUdc, const EcString name, ubyte_t value);

__LIB_EXPORT void ecudc_add_asL (EcUdc, const EcString name, ulong_t value);

__CPP_EXTERN______________________________________________________________________________END

#endif

