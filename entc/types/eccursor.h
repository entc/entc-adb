/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

#ifndef ENTC_TYPES_CURSOR_H
#define ENTC_TYPES_CURSOR_H 1

#include "sys/entc_export.h"
#include "types/ectable.h"

struct EcCursor_s; typedef struct EcCursor_s* EcCursor;

// callback declarations

typedef int  (__STDCALL *eccursor_fill_fct)(void* ptr, EcTable*);

typedef int  (__STDCALL *eccursor_destroy_fct)(void* ptr, EcTable);


__ENTC_LIBEX
  
__ENTC_LIBEX EcCursor eccursor_create (void);
  
__ENTC_LIBEX void eccursor_destroy (EcCursor*);
  
__ENTC_LIBEX int eccursor_next (EcCursor);

__ENTC_LIBEX void* eccursor_get (EcCursor, int column);

// header

__ENTC_LIBEX int eccursor_cols (EcCursor);

__ENTC_LIBEX void* eccursor_header (EcCursor, int column);

// callback 

__ENTC_LIBEX void eccursor_callbacks (EcCursor, void* ptr, eccursor_fill_fct, eccursor_destroy_fct);



#endif
