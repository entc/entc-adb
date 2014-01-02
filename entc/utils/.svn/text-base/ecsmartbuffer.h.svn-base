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

#ifndef ENTC_UTILS_SMARTBUFFER_H
#define ENTC_UTILS_SMARTBUFFER_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecstring.h"

#include "../utils/eclogger.h"
#include "../system/ecfile.h"

struct EcSmartbuffer_s; typedef struct EcSmartbuffer_s* EcSmartbuffer;

__CPP_EXTERN______________________________________________________________________________START
  
  
__LIB_EXPORT EcSmartbuffer ecsmartbuffer_new(uint_t max);

__LIB_EXPORT void ecsmartbuffer_delete(EcSmartbuffer);

__LIB_EXPORT void ecsmartbuffer_setFile(EcSmartbuffer, const EcString filename, const EcString confdir, EcLogger logger);

__LIB_EXPORT void ecsmartbuffer_append(EcSmartbuffer, const EcString start, uint_t length, uint_t keep);

__LIB_EXPORT void ecsmartbuffer_flush(EcSmartbuffer, uint_t keep);

__LIB_EXPORT char* ecsmartbuffer_data(EcSmartbuffer, uint_t n);

__LIB_EXPORT char* ecsmartbuffer_copy(EcSmartbuffer);

__LIB_EXPORT void ecsmartbuffer_reduce(EcSmartbuffer, uint_t n);

__LIB_EXPORT int ecsmartbuffer_isEmpty(EcSmartbuffer);

__LIB_EXPORT int ecsmartbuffer_isEqual(EcSmartbuffer, const EcString to);

__LIB_EXPORT void ecsmartbuffer_setCompareKey(EcSmartbuffer, const EcString key);
//returns the remain bytes to compare
//if 0 then the comparison is done
__LIB_EXPORT uint_t ecsmartbuffer_getCompareStatus(EcSmartbuffer);
//return the result of comparison
__LIB_EXPORT int ecsmartbuffer_getCompareEqual(EcSmartbuffer);

__CPP_EXTERN______________________________________________________________________________END

#endif
