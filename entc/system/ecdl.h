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

#ifndef ENTC_SYSTEM_DL_H
#define ENTC_SYSTEM_DL_H 1

#include "sys/entc_export.h"
#include "types/ecstring.h"

struct EcLibraryHandle_s; typedef struct EcLibraryHandle_s* EcLibraryHandle;

  
__ENTC_LIBEX EcLibraryHandle ecdl_new(const EcString filename);

__ENTC_LIBEX EcLibraryHandle ecdl_fromName(const EcString path, const EcString name);

__ENTC_LIBEX void ecdl_delete(EcLibraryHandle*);
  
__ENTC_LIBEX void* ecdl_method(EcLibraryHandle, const EcString name);


#endif
