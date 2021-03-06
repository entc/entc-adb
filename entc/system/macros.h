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

#ifndef ENTC_SYSTEM_MACROS_H
#define ENTC_SYSTEM_MACROS_H 1

#include "platform.h"

#ifdef ENTC_PLATFORM_WINDOWS
#define __LIB_EXPORT  __declspec( dllexport )
#define _STDCALL __stdcall
#else
#define __LIB_EXPORT
#define _STDCALL
#endif

#define TRUE 1
#define FALSE 0


#ifdef ENTC_PLATFORM_DOS

#if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
#include <malloc.h>
#else
#include <alloc.h>
#endif

#include <mem.h>

#define ENTC_MALLOC( size ) malloc(size)
#define ENTC_FREE(ptr) free(ptr)

#define ENTC_NEW( type ) (type*)malloc(sizeof(type))
#define ENTC_DEL( ptr, type ) { memset(*ptr, 0, sizeof(type)); free(*ptr); *ptr = 0; }

#else

#include <memory.h>
#include <stdlib.h>

#define ENTC_MALLOC( size ) malloc(size)
#define ENTC_FREE(ptr) free(ptr)

#define ENTC_NEW( type ) (type*)malloc(sizeof(type))
#define ENTC_DEL( ptr, type ) { memset(*ptr, 0, sizeof(type)); free(*ptr); *ptr = 0; }

#endif


#ifdef __cplusplus

  #define __CPP_EXTERN______________________________________________________________________________START extern "C" {
  #define __CPP_EXTERN______________________________________________________________________________END }

#else

#define __CPP_EXTERN______________________________________________________________________________START
#define __CPP_EXTERN______________________________________________________________________________END

#endif

#endif
