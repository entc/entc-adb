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

#ifndef ENTC_SYSTEM_PROC_H
#define ENTC_SYSTEM_PROC_H 1

//-----------------------------------------------------------------------------

// entc inlcudes
#include "types/ecerr.h"
#include "types/ecstring.h"

//-----------------------------------------------------------------------------

struct EcProc_s; typedef struct EcProc_s* EcProc;

//-----------------------------------------------------------------------------

__LIBEX EcProc ecproc_create (void);

__LIBEX void ecproc_destroy (EcProc*);

__LIBEX EcProc ecproc_get (int argc, char* argv[], EcErr err);

__LIBEX int ecproc_start (EcProc, const char* command, const char* args, EcErr err);

//-----------------------------------------------------------------------------

__LIBEX void* ecproc_handle (EcProc);

__LIBEX void ecproc_terminate (EcProc);

__LIBEX int ecproc_waitForProcessToTerminate (EcProc, EcErr);

__LIBEX void* ecproc_getHandleIn (EcProc);

__LIBEX void* ecproc_getHandleOut (EcProc);

//-----------------------------------------------------------------------------

__LIBEX EcString ecproc_getExecutableName (int argc, char* argv[]);

__LIBEX int ecproc_waitForProcess (void* handle, EcErr);

__LIBEX void ecproc_terminateProcess (void* handle);

//-----------------------------------------------------------------------------

#endif
