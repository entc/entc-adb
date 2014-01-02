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

#ifndef ENTC_TYPES_MAPCHAR_H
#define ENTC_TYPES_MAPCHAR_H 1

/* include external macro for win32 */
#include "../system/macros.h"

#include "../types/ecstring.h"
#include "../types/eclist.h"

struct EcMapChar_s;

typedef struct EcMapChar_s* EcMapChar;

typedef struct EcListNode_s* EcMapCharNode;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcMapChar ecmapchar_new();
  
__LIB_EXPORT void ecmapchar_delete(EcMapChar*);
  
__LIB_EXPORT void ecmapchar_clear(EcMapChar);

__LIB_EXPORT void ecmapchar_set(EcMapChar, const EcString key, const EcString value);

__LIB_EXPORT void ecmapchar_append(EcMapChar, const EcString key, const EcString value);

__LIB_EXPORT void ecmapchar_appendTO(EcMapChar, const EcString key, EcString value);

__LIB_EXPORT EcMapCharNode ecmapchar_find(EcMapChar, const EcString key);

__LIB_EXPORT const EcString ecmapchar_finddata(EcMapChar, const EcString key);
  
__LIB_EXPORT EcMapCharNode ecmapchar_first(const EcMapChar);
  
__LIB_EXPORT EcMapCharNode ecmapchar_next(const EcMapCharNode);

__LIB_EXPORT EcMapCharNode ecmapchar_end(const EcMapChar);
  
__LIB_EXPORT const EcString ecmapchar_key(const EcMapCharNode);
  
__LIB_EXPORT const EcString ecmapchar_data(const EcMapCharNode);
  
__LIB_EXPORT void ecmapchar_copy(EcMapChar dest, const EcMapChar source);
  
__LIB_EXPORT uint_t ecmapchar_count(const EcMapChar);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
