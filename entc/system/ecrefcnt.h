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

#ifndef ENTC_SYSTEM_REFCNT_H
#define ENTC_SYSTEM_REFCNT_H 1

#include "sys/entc_export.h"

struct EcRefCnt_s; typedef struct EcRefCnt_s* EcRefCnt;

__ENTC_LIBEX EcRefCnt ecrefcnt_new (void*);

__ENTC_LIBEX void ecrefcnt_delete (EcRefCnt*);

__ENTC_LIBEX void ecrefcnt_inc (EcRefCnt);

__ENTC_LIBEX int ecrefcnt_dec (EcRefCnt);

__ENTC_LIBEX void* ecrefcnt_get (EcRefCnt);

#endif
