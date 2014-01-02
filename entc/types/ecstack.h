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

#ifndef ENTC_TYPES_STACK_H
#define ENTC_TYPES_STACK_H 1

#include "../system/macros.h"

struct EcStack_s; typedef struct EcStack_s* EcStack;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcStack ecstack_new();
  
__LIB_EXPORT void ecstack_delete(EcStack*);
    
__LIB_EXPORT void ecstack_clear(EcStack);
  
__LIB_EXPORT void ecstack_push(EcStack, void* data);
  
  /* delete the top element, returns false no element anymore */
__LIB_EXPORT int ecstack_pop(EcStack);
  /* get the data from the top element */
__LIB_EXPORT void* ecstack_top(EcStack);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
