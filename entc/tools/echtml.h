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

#ifndef ENTC_TOOLS_HTML_H
#define ENTC_TOOLS_HTML_H 1

/* include external macro for win32 */
#include "../system/macros.h"
#include "../system/ecevents.h"

#include "../types/ecstring.h"
#include "../types/ecstream.h"

#include "../utils/eclogger.h"

struct EcHtmlRequest_s; typedef struct EcHtmlRequest_s* EcHtmlRequest;

__CPP_EXTERN______________________________________________________________________________START 

__LIB_EXPORT EcHtmlRequest echtmlreq_new (void);
  
__LIB_EXPORT void echtmlreq_delete(EcHtmlRequest*);
  
__LIB_EXPORT int echtmlreq_get (EcHtmlRequest, const EcString host, uint_t port, const EcString url, EcEventContext);
  
__LIB_EXPORT int echtmlreq_post (EcHtmlRequest, const EcString host, uint_t port, const EcString url, const EcString message, EcEventContext);

__LIB_EXPORT void echtmlreq_escapeUrl(EcStream stream, const EcString source);

__LIB_EXPORT const EcString echtmlreq_data(EcHtmlRequest);
  
__CPP_EXTERN______________________________________________________________________________END

#endif
