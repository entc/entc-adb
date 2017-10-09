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

#ifndef ENTC_TOOLS_MIME_H
#define ENTC_TOOLS_MIME_H 1

//-----------------------------------------------------------------------------

// entc includes
#include "types/ecerr.h"
#include "types/ecstring.h"
#include "types/ecmapchar.h"
#include "types/ecbuffer.h"

//=============================================================================

__LIBEX const EcString ecmime_getFromFile (const EcString filename);

__LIBEX const EcString ecmime_getFromExtension (const EcString ext);

__LIBEX void ecmime_unescape (EcString url);

//=============================================================================

struct EcMultipartParser_s; typedef struct EcMultipartParser_s* EcMultipartParser;

//-----------------------------------------------------------------------------

typedef void   (__STDCALL *ecmultipartparser_callback)  (void* ptr, EcBuffer*, EcMapChar*);
typedef char*  (__STDCALL *http_content_callback)       (void* ptr, char* buffer, ulong_t inSize, ulong_t* outRes);

//-----------------------------------------------------------------------------

__LIBEX EcMultipartParser ecmultipartparser_create (const EcString boundary, const EcString, http_content_callback cb, void* ptr, ecmultipartparser_callback dc, void* obj);

__LIBEX void ecmultipartparser_destroy (EcMultipartParser*);

__LIBEX int ecmultipartparser_process (EcMultipartParser, ulong_t size);

__LIBEX EcString echttpheader_parseLine (const EcString line, const EcString key);

__LIBEX void echttpheader_parseParam (EcMapChar map, const EcString line);

//=============================================================================

struct EcMultipart_s; typedef struct EcMultipart_s* EcMultipart;

//-----------------------------------------------------------------------------

__LIBEX EcMultipart ecmultipart_create (const EcString boundary, const EcString header);

__LIBEX void ecmultipart_destroy (EcMultipart*);

__LIBEX void ecmultipart_addText (EcMultipart, const EcString text, const EcString mimeType);

__LIBEX void ecmultipart_addFile (EcMultipart, const EcString path, const EcString file, int fileId);

__LIBEX void ecmultipart_addPath (EcMultipart, const EcString path, const EcString name, int fileId);

__LIBEX void ecmultipart_addContentDisposition_B_o (EcMultipart, const EcString name, EcBuffer*);

__LIBEX void ecmultipart_addContentDisposition_S (EcMultipart, const EcString name, const EcString content);

__LIBEX void ecmultipart_addContentDisposition_S_o (EcMultipart, const EcString name, EcString* content);

__LIBEX EcString ecmultipart_startGetContentType (EcMultipart);

__LIBEX uint_t ecmultipart_next (EcMultipart, EcBuffer);

//-----------------------------------------------------------------------------

#endif
