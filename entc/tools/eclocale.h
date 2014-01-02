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

#ifndef ENTC_TOOLS_LOCALE_H
#define ENTC_TOOLS_LOCALE_H 1

#include "../system/macros.h"
#include "../system/ecevents.h"

#include "../types/ecstring.h"
#include "../types/ecmap.h"

struct EcLocale_s
{
  
  EcMap languages;
  
};

typedef struct EcLocale_s* EcLocale;

struct EcLocaleSet_s; typedef struct EcLocaleSet_s* EcLocaleSet;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcLocale eclocale_new(const EcString confdir, const EcString path, EcEventFiles events , EcLogger logger);
  
__LIB_EXPORT void eclocale_delete(EcLocale* ptr);

__LIB_EXPORT EcLocaleSet eclocale_getSet(EcLocale, const EcString);
  
__LIB_EXPORT const EcString eclocaleset_get(EcLocaleSet, const EcString);
  
__LIB_EXPORT EcLocaleSet eclocale_getDefaultSet(EcLocale);
  
__LIB_EXPORT const EcString eclocale_lang(EcLocaleSet);

__LIB_EXPORT const EcString eclocale_desc(EcLocaleSet);

__CPP_EXTERN______________________________________________________________________________END

#endif
