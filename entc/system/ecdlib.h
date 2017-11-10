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

#ifndef ENTC_SYSTEM_DLIB_H
#define ENTC_SYSTEM_DLIB_H 1

#include "types/ecerr.h"
#include "types/ecstring.h"

//=============================================================================

struct EcDl_s; typedef struct EcDl_s* EcDl;

//-----------------------------------------------------------------------------

__LIBEX EcDl ecdl_create (const EcString name, const EcString path);

__LIBEX void ecdl_destroy (EcDl*);

__LIBEX int ecdl_load (EcDl, EcErr);

__LIBEX int ecdl_unload (EcDl, EcErr);

__LIBEX int ecdl_assign (EcDl, EcErr, void* buffer, int n, ...);

//=============================================================================

#endif
