/*
 * Copyright (c) 2010-2019 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef __ENTC_TOOLS__MULTIPART_H
#define __ENTC_TOOLS__MULTIPART_H 1

//-----------------------------------------------------------------------------

#include "types/ecerr.h"
#include "types/ecstring.h"
#include "types/ecbuffer.h"

//=============================================================================

struct EntcMultipart_s; typedef struct EntcMultipart_s* EntcMultipart;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcMultipart   entc_multipart_new          (const EcString boundary, const EcString header);

__ENTC_LIBEX   void            entc_multipart_del          (EntcMultipart*);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void            entc_multipart_add_text     (EntcMultipart, const EcString text, const EcString mimeType);

__ENTC_LIBEX   void            entc_multipart_add_file     (EntcMultipart, const EcString path, const EcString file, int fileId, const EcString vsec, unsigned int aes_type);

__ENTC_LIBEX   void            entc_multipart_add_path     (EntcMultipart, const EcString path, const EcString name, int fileId, const EcString vsec, unsigned int aes_type);

__ENTC_LIBEX   void            entc_multipart_add_buf_ot   (EntcMultipart, const EcString name, EcBuffer*);                  // add ContentDisposition

__ENTC_LIBEX   void            entc_multipart_add_str      (EntcMultipart, const EcString name, const EcString content);     // add ContentDisposition

__ENTC_LIBEX   void            entc_multipart_add_str_ot   (EntcMultipart, const EcString name, EcString* content);          // add ContentDisposition

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EcString        entc_multipart_content_type (EntcMultipart);

__ENTC_LIBEX   uint_t          entc_multipart_next         (EntcMultipart, EcBuffer);   // min buffer size == 200 -> for base64 encoding

//-----------------------------------------------------------------------------

#endif
