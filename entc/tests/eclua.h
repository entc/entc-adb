/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TESTS_LUA_H
#define ENTC_TESTS_LUA_H 1

//=============================================================================

#include "system/ecdefs.h"
#include "types/ecstring.h"
#include "types/ecerr.h"
#include "types/ecstream.h"

//=============================================================================

struct EcLua_s; typedef struct EcLua_s* EcLua;

//-----------------------------------------------------------------------------

__LIBEX EcLua eclua_create ();

__LIBEX void eclua_destroy (EcLua*);

__LIBEX int eclua_init (EcLua, EcErr err);

//-----------------------------------------------------------------------------

#endif
