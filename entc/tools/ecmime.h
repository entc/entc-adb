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

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT const EcString ecmime_getFromFile (const EcString filename);

__LIB_EXPORT const EcString ecmime_getFromExtension (const EcString ext);

__CPP_EXTERN______________________________________________________________________________END

struct EcHttpContent_s;
struct EcMultipartParser_s; typedef struct EcMultipartParser_s* EcMultipartParser;

typedef void (_STDCALL *ecmultipartparser_callback) (void* ptr, EcBuffer*, EcMapChar*);

__CPP_EXTERN______________________________________________________________________________START
  
__LIB_EXPORT EcMultipartParser ecmultipartparser_create (const EcString boundary, const EcString, http_content_callback cb, void* ptr, struct EcHttpContent_s*, ecmultipartparser_callback dc, void* obj);

__LIB_EXPORT void ecmultipartparser_destroy (EcMultipartParser*);

__LIB_EXPORT int ecmultipartparser_process (EcMultipartParser, ulong_t size);

__LIB_EXPORT EcString echttpheader_parseLine (const EcString line, const EcString key);

__LIB_EXPORT void echttpheader_parseParam (EcMapChar map, const EcString line);

__CPP_EXTERN______________________________________________________________________________END

struct EcMultipart_s; typedef struct EcMultipart_s* EcMultipart;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcMultipart ecmultipart_create (const EcString boundary, const EcString header);

__LIB_EXPORT void ecmultipart_destroy (EcMultipart*);

__LIB_EXPORT void ecmultipart_addText (EcMultipart, const EcString text, const EcString mimeType);

__LIB_EXPORT void ecmultipart_addFile (EcMultipart, const EcString path, const EcString file, int fileId);

__LIB_EXPORT void ecmultipart_addPath (EcMultipart, const EcString path, const EcString name, int fileId);

__LIB_EXPORT void ecmultipart_addContentDisposition_B_o (EcMultipart, const EcString name, EcBuffer*);

__LIB_EXPORT void ecmultipart_addContentDisposition_S (EcMultipart, const EcString name, const EcString content);

__LIB_EXPORT void ecmultipart_addContentDisposition_S_o (EcMultipart, const EcString name, EcString* content);

__LIB_EXPORT EcString ecmultipart_startGetContentType (EcMultipart);

__LIB_EXPORT uint_t ecmultipart_next (EcMultipart, EcBuffer);

__CPP_EXTERN______________________________________________________________________________END

#endif
