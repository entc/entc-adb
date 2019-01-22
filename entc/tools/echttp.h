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

#ifndef ENTC_TOOLS_HTTP_H
#define ENTC_TOOLS_HTTP_H 1

#include <system/macros.h>
#include <system/types.h>
#include <system/ecsocket.h>

#include <types/ecstring.h>
#include <types/ecstream.h>
#include <types/ecmapchar.h>

#include <utils/ecstreambuffer.h>

#define Q5_MSGTYPE_HTTP_REQUEST_INFO      14
#define Q5_SERVICE_HTTP_REQUEST_INFO    3002

struct EcHttpContent_s; typedef struct EcHttpContent_s* EcHttpContent;

typedef char* __ENTC_LIBEX *http_content_callback) (void* ptr, char* buffer, ulong_t inSize, ulong_t* outRes);

__ENTC_LIBEX

__ENTC_LIBEX EcHttpContent echttp_content_create (ulong_t size, const EcString type, http_content_callback bf, http_content_callback mm, void*, const EcString path);

__ENTC_LIBEX EcHttpContent echttp_content_create2 (EcBuffer* pbuf, EcMapChar* pparams);

__ENTC_LIBEX void echttp_content_destroy (EcHttpContent*);

__ENTC_LIBEX int echttp_content_hasBuffer (EcHttpContent);

__ENTC_LIBEX int echttp_content_hasFile (EcHttpContent);

__ENTC_LIBEX EcString echttp_content_getFile (EcHttpContent);

__ENTC_LIBEX EcBuffer echttp_content_getBuffer (EcHttpContent);

__ENTC_LIBEX EcHttpContent echttp_content_add (EcHttpContent, EcHttpContent*);

__ENTC_LIBEX EcHttpContent echttp_content_next (EcHttpContent);

__ENTC_LIBEX const EcString echttp_content_parameter (EcHttpContent, const EcString name);

__ENTC_LIBEX EcString echttp_content_extractString (EcHttpContent);

__ENTC_LIBEX EcBuffer echttp_content_extractBuffer (EcHttpContent);

__ENTC_LIBEX

typedef struct {

  int header_on;
  
  int header_only;
  
  EcString method;
    
  EcString host;
  
  const EcString remote_address;
  
  const EcString mime;
  
  EcString url;
  
  EcString user_lang;
  
  EcString user_agent;
  
  EcString session_lang;
  
  EcString request_params;
  
  EcString request_url;
  
  EcString title;
  
  EntcList tokens;
  
  EntcListNode token;
  
  EcString urlpath;
  
  // *** content
  
  uint_t content_length;
  
  EcHttpContent content;

  EcString content_type;
  
  EcString sessionid;
  
  EcUdc auth;
  
  EcMapChar values;
  
} EcHttpHeader;

struct EcHttpRequest_s; typedef struct EcHttpRequest_s* EcHttpRequest;

typedef int __ENTC_LIBEX *http_route_fct)(void* ptr, EcHttpHeader*, void** object);
typedef int __ENTC_LIBEX *http_validate_header_fct)(void* ptr, EcHttpHeader*, EcDevStream, void** object);
typedef int __ENTC_LIBEX *http_render_fct)(void* ptr, EcHttpHeader*, EcDevStream, void** object);
typedef int __ENTC_LIBEX *http_header_fct)(void* ptr, EcHttpHeader*);
typedef int __ENTC_LIBEX *http_content_fct)(void* ptr, EcHttpHeader*, const EcString tmproot);
typedef int __ENTC_LIBEX *http_clear_fct)(void* ptr, void** object);

typedef struct {
  
  http_route_fct route;
  
  http_validate_header_fct validate;
  
  http_render_fct render;
  
  http_header_fct custom_header;
  
  http_content_fct content;
  
  http_clear_fct clear;
  
  void* ptr;

} EcHttpCallbacks;

/*
typedef int (*http_callback_init)(void* ptr, void** pobj, EcHttpRequest*);
typedef int (*http_callback_done)(void* ptr, void** pobj, EcHttpRequest*);
typedef int (*http_callback_render)(void* ptr, void* obj, EcHttpRequest*, EcDevStream, EcLogger);
*/
 

__ENTC_LIBEX

__ENTC_LIBEX void echttp_init (void);

__ENTC_LIBEX void echttp_done (void);

__ENTC_LIBEX EcHttpRequest echttp_request_create (const EcString docroot, const EcString tmproot, int header);

__ENTC_LIBEX void echttp_request_destroy (EcHttpRequest*);

__ENTC_LIBEX void echttp_request_process (EcHttpRequest, EcSocket);

__ENTC_LIBEX void echttp_request_process_dev (EcHttpRequest, EcDevStream, void* callback_ptr);

__ENTC_LIBEX void echttp_request_callbacks (EcHttpRequest, EcHttpCallbacks*);

__ENTC_LIBEX void echttp_parse_cookies (EcHttpHeader*, const EcString s);

__ENTC_LIBEX EcUdc echttp_parse_auth (const EcString source);

// misc

__ENTC_LIBEX void echttp_unescape (EcString url);

__ENTC_LIBEX void echttp_escape (EcDevStream stream, const EcString url);

__ENTC_LIBEX void echttp_escape_stream (EcStream stream, const EcString url);

__ENTC_LIBEX const EcString echttp_url_lastPart (const EcString url);

__ENTC_LIBEX void echttp_url (EcHttpHeader* header, EcDevStream stream, const EcString url);

__ENTC_LIBEX void echttp_realurl (EcHttpHeader* header, EcDevStream stream, const EcString url);

__ENTC_LIBEX void echttp_send_header (EcHttpHeader* header, EcDevStream stream, const EcString code, EcUdc extra_params);

__ENTC_LIBEX void echttp_send_ErrHeader (EcHttpHeader* header, EcDevStream stream, ulong_t errcode, EcUdc extra_params);

__ENTC_LIBEX void echttp_send_DefaultHeader (EcHttpHeader* header, EcDevStream stream, EcUdc extra_params);

__ENTC_LIBEX const EcString echttp_getMimeType (const EcString filename);

__ENTC_LIBEX EcUdc echttp_getParams (EcHttpHeader* header);

__ENTC_LIBEX

#endif
