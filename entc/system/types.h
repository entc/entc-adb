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

#ifndef ENTC_SYSTEM_TYPES_H
#define ENTC_SYSTEM_TYPES_H 1

typedef unsigned char ubyte_t;

typedef unsigned long ulong_t;
typedef long long_t;


typedef unsigned int uint_t;
typedef signed int int_t;

#define ENTC_RESOURCE_UNKNOWN            0
#define ENTC_RESOURCE_AVAILABLE          1
#define ENTC_RESOURCE_NOT_FOUND          2
#define ENTC_RESOURCE_NEEDS_AUTH         3
#define ENTC_RESOURCE_NEEDS_PERMISSION   4
#define ENTC_RESOURCE_ALREADY_EXISTS     5

#define isAssigned(pointer) (pointer != NULL)
#define isNotAssigned(pointer) (pointer == NULL)

#endif
