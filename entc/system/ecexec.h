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

#ifndef ENTC_SYSTEM_EXEC_H
#define ENTC_SYSTEM_EXEC_H 1

//-----------------------------------------------------------------------------

#include "system/ecdefs.h"
#include "types/ecstring.h"

//=============================================================================

struct EcExec_s; typedef struct EcExec_s* EcExec;

//-----------------------------------------------------------------------------

__LIBEX EcExec ecexec_new (const EcString script);

__LIBEX void ecexec_delete (EcExec*);

__LIBEX void ecexec_addParameter (EcExec, const EcString);

__LIBEX int ecexec_run (EcExec);

__LIBEX const EcString ecexec_stdout (EcExec);

__LIBEX const EcString ecexec_stderr (EcExec);

//-----------------------------------------------------------------------------

#endif
