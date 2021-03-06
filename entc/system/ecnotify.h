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

#ifndef ENTC_SYSTEM_EVENTS_H
#define ENTC_SYSTEM_EVENTS_H 1

#include "../system/macros.h"
#include "../types/ecstring.h"

struct EcNotify_s; typedef struct EcNotify_s* EcNotify;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcNotify ecnotify_create ();
  
__LIB_EXPORT void ecnotify_destroy (EcNotify*);

__LIB_EXPORT void enotify_addPath (EcNotify, const EcString path);

__CPP_EXTERN______________________________________________________________________________END


#endif
