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

#ifndef ENTC_SYSTEM_DEFS_H
#define ENTC_SYSTEM_DEFS_H 1

#ifdef __cplusplus
#define __EXTERN_C    extern "C"
#else
#define __EXTERN_C
#endif

#if defined _WIN64 || defined _WIN32

#define __WIN_OS
#define __LIBEX __EXTERN_C __declspec(dllexport)

#define __STDCALL __stdcall

#elif defined __APPLE__

#define __BSD_OS

#define __LIBEX __EXTERN_C
#define __STDCALL

#elif __linux__

#define __LINUX_OS

#define __LIBEX __EXTERN_C
#define __STDCALL

#endif

#endif
