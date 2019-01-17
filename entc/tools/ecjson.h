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

#ifndef ENTC_TOOLS_JSON_H
#define ENTC_TOOLS_JSON_H 1

//=============================================================================

#include "sys/entc_export.h"
#include "types/ecudc.h"
#include "types/ecstring.h"

//-----------------------------------------------------------------------------

__ENTC_LIBEX EcUdc ecjson_read_s (const EcString, const EcString name);

__ENTC_LIBEX EcUdc ecjson_read_ecbuf (const EcBuffer, const EcString name);

__ENTC_LIBEX EcUdc ecjson_read_buffer (const char* buffer, int64_t size, const EcString name);

__ENTC_LIBEX EcBuffer ecjson_write (const EcUdc source);

__ENTC_LIBEX EcString ecjson_toString (const EcUdc source);

__ENTC_LIBEX int ecjson_readFromFile (const EcString filename, EcUdc*, const EcString secret, unsigned int sectype);

__ENTC_LIBEX int ecjson_writeToFile (const EcString filename, const EcUdc, const EcString secret, unsigned int sectype);

//-----------------------------------------------------------------------------

#endif
