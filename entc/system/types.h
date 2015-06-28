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

#if defined _WIN64 || defined _WIN32

typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t; 

#else
#include <stdint.h>
#endif

typedef unsigned char ubyte_t;

typedef unsigned long ulong_t;
typedef long long_t;

typedef unsigned int uint_t;
typedef signed int int_t;


#define ENTC_RESCODE_UNKNOWN_ERROR       0
#define ENTC_RESCODE_OK                  1
#define ENTC_RESCODE_IGNORE              2
#define ENTC_RESCODE_INPUT_ERROR         3

#define ENTC_RESCODE_NOT_FOUND           10
#define ENTC_RESCODE_NOT_AVAILABLE       11

#define ENTC_RESCODE_NEEDS_AUTH          21
#define ENTC_RESCODE_NEEDS_PERMISSION    22
#define ENTC_RESCODE_ALREADY_EXISTS      23
#define ENTC_RESCODE_CLEAR_AUTH          24


#define ENTC_RESOURCE_UNKNOWN            0
#define ENTC_RESOURCE_AVAILABLE          1
#define ENTC_RESOURCE_NOT_FOUND          2
#define ENTC_RESOURCE_NEEDS_AUTH         3
#define ENTC_RESOURCE_NEEDS_PERMISSION   4
#define ENTC_RESOURCE_ALREADY_EXISTS     5

#define isAssigned(pointer) (pointer != NULL)
#define isNotAssigned(pointer) (pointer == NULL)

#define ENTC_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ENTC_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#endif
