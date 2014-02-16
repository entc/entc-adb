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

struct EcUdc_s; typedef struct EcUdc_s* EcUdc;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcUdc ecudc_new (uint_t type, const EcString name);

__LIB_EXPORT void ecudc_del (EcUdc*);

__LIB_EXPORT void ecudc_add (EcUdc, EcUdc*);

__LIB_EXPORT EcUdc ecudc_get (EcUdc, const EcString name);

__LIB_EXPORT void ecudc_setS (EcUdc, const EcString value);

__LIB_EXPORT const EcString ecudc_asString (EcUdc);

__CPP_EXTERN______________________________________________________________________________END

#endif

