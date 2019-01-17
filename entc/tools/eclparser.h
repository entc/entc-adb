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

#ifndef ENTC_TOOLS_LPARSER_H
#define ENTC_TOOLS_LPARSER_H 1

//=============================================================================

#include "sys/entc_export.h"
#include "types/ecstring.h"

//-----------------------------------------------------------------------------

struct EcLineParser_s; typedef struct EcLineParser_s* EcLineParser;

typedef void (__STDCALL *fct_eclineparser_onLine) (void* ptr, const EcString line);

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcLineParser eclineparser_create (fct_eclineparser_onLine onLine, void* ptr);

__ENTC_LIBEX void eclineparser_destroy (EcLineParser* pself);

__ENTC_LIBEX void eclineparser_parse (EcLineParser, const char* buffer, int size, int last);

//-----------------------------------------------------------------------------

#endif
