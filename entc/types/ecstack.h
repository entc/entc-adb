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

#include "sys/entc_export.h"

struct EcStack_s; typedef struct EcStack_s* EcStack;

__ENTC_LIBEX EcStack ecstack_new();
  
__ENTC_LIBEX void ecstack_delete(EcStack*);
    
__ENTC_LIBEX void ecstack_clear(EcStack);
  
__ENTC_LIBEX void ecstack_push(EcStack, void* data);
  
  /* delete the top element, returns false no element anymore */
  __ENTC_LIBEX int ecstack_pop(EcStack);
  /* get the data from the top element */
  __ENTC_LIBEX void* ecstack_top(EcStack);
  
#endif
