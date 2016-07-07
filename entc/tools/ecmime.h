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

#ifndef ENTC_TOOLS_MIME_H
#define ENTC_TOOLS_MIME_H 1

#include "system/macros.h"
#include "types/ecstring.h"
#include "tools/echttp.h"

struct EcMultipartParser_s; typedef struct EcMultipartParser_s* EcMultipartParser;

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcMultipartParser ecmultipartparser_create (const EcString boundary, http_content_callback cb, void* ptr);

__LIB_EXPORT void ecmultipartparser_destroy (EcMultipartParser*);

__LIB_EXPORT int ecmultipartparser_process (EcMultipartParser, const EcString path, ulong_t size);

__LIB_EXPORT EcString echttpheader_parseLine (const EcString line, const EcString key);

__CPP_EXTERN______________________________________________________________________________END

#endif
