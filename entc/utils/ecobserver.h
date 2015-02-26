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

#ifndef ENTC_UTILS_OBSERVER_H
#define ENTC_UTILS_OBSERVER_H 1

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/ecstring.h"

#include "../system/ecevents.h"
#include "../system/ecfile.h"

struct EcFileObserver_s;

struct EcDirObserver_s;

typedef struct EcFileObserver_s* EcFileObserver;

typedef struct EcDirObserver_s* EcDirObserver;

__CPP_EXTERN______________________________________________________________________________START

  /* needs a dir filedescriptor, the descriptor is closed after usage */
__LIB_EXPORT EcFileObserver ecf_observer_newFromPath(const EcString path, const EcString filename, const EcString confdir, EcEventFiles, events_callback_fct, void*);
  
__LIB_EXPORT EcFileObserver ecf_observer_new(const EcString filename, const EcString confdir, EcEventFiles, events_callback_fct, void*);
  
__LIB_EXPORT void ecf_observer_delete(EcFileObserver);
  
__LIB_EXPORT EcFileHandle ecf_observer_open(EcFileObserver);
  
__LIB_EXPORT void ecf_observer_close(EcFileObserver);
  
  //_EXPORT int ecf_observer_check(EcFileObserver);
  
__LIB_EXPORT int ecf_observer_exists(EcFileObserver);
  
__LIB_EXPORT const EcString ecf_observer_getFileName(EcFileObserver);
  
  
  /* needs a dir filedescriptor, the descriptor is closed after usage */
__LIB_EXPORT EcDirObserver ecd_observer_new(const EcString path, EcEventFiles);
  
__LIB_EXPORT void ecd_observer_delete(EcDirObserver);
  
__LIB_EXPORT EcDirHandle ecd_observer_open(EcDirObserver, const EcString confdir);
  
__LIB_EXPORT void ecd_observer_close(EcDirObserver);
  
  //_EXPORT int ecd_observer_check(EcDirObserver);
  
__LIB_EXPORT const EcString ecd_observer_getDirName(EcDirObserver);

__CPP_EXTERN______________________________________________________________________________END

#endif
