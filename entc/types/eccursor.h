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

#include "system/macros.h"
#include "system/types.h"
#include "types/ectable.h"

struct EcCursor_s; typedef struct EcCursor_s* EcCursor;

// callback declarations

typedef int  (_STDCALL *eccursor_fill_fct)(void* ptr, EcTable*);

typedef int  (_STDCALL *eccursor_destroy_fct)(void* ptr, EcTable);


__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcCursor eccursor_create (void);
  
__LIB_EXPORT void eccursor_destroy (EcCursor*);
  
__LIB_EXPORT int eccursor_next (EcCursor);

__LIB_EXPORT void* eccursor_get (EcCursor, int column);

// header

__LIB_EXPORT int eccursor_cols (EcCursor);

__LIB_EXPORT void* eccursor_header (EcCursor, int column);

// callback 

__LIB_EXPORT void eccursor_callbacks (EcCursor, void* ptr, eccursor_fill_fct, eccursor_destroy_fct);

__CPP_EXTERN______________________________________________________________________________END

#endif
