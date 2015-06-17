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

typedef char* (_STDCALL *http_content_callback) (void* ptr, char* buffer, ulong_t inSize, int* outRes);

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcHttpContent echttp_content_create (ulong_t size, http_content_callback bf, http_content_callback mm, void*, const EcString path);

__LIB_EXPORT void echttp_content_destroy (EcHttpContent*);

__LIB_EXPORT int echttp_content_hasBuffer (EcHttpContent);

__LIB_EXPORT int echttp_content_hasFile (EcHttpContent);

__LIB_EXPORT EcString echttp_content_getFile (EcHttpContent);

__LIB_EXPORT EcBuffer echttp_content_getBuffer (EcHttpContent);

__CPP_EXTERN______________________________________________________________________________END

typedef struct {

  int header_on;
  
  int header_only;
  
  EcString method;
    
  EcString host;
  
  const EcString remote_address;
  
  const EcString mime;
  
  const EcString url;
  
  EcString user_lang;
  
  EcString user_agent;
  
  EcString session_lang;
  
  EcString request_params;
  
  EcString request_url;
  
  EcString title;
  
  EcList tokens;
  
  EcListNode token;
  
  EcString urlpath;
  
  uint_t content_length;
  
  EcHttpContent content;
  
  EcString sessionid;
  
  EcUdc auth;
  
  EcMapChar values;
  
} EcHttpHeader;

struct EcHttpRequest_s; typedef struct EcHttpRequest_s* EcHttpRequest;

typedef int (_STDCALL *http_route_fct)(void* ptr, EcHttpHeader*, void** object);
typedef int (_STDCALL *http_validate_header_fct)(void* ptr, EcHttpHeader*, EcDevStream, void** object);
typedef int (_STDCALL *http_render_fct)(void* ptr, EcHttpHeader*, EcDevStream, void** object);
typedef int (_STDCALL *http_header_fct)(void* ptr, EcHttpHeader*);
typedef int (_STDCALL *http_content_fct)(void* ptr, EcHttpHeader*, const EcString tmproot);

typedef struct {
  
  http_route_fct cb_route;
  
  http_validate_header_fct cb_validate;
  
  http_render_fct render;

  void* process_ptr;
  
  void* render_ptr; 
  
  // needed for custom-dev processing
  
  http_header_fct custom_header;
  
  http_content_fct content;
  
} EcHttpCallbacks;

/*
typedef int (*http_callback_init)(void* ptr, void** pobj, EcHttpRequest*);
typedef int (*http_callback_done)(void* ptr, void** pobj, EcHttpRequest*);
typedef int (*http_callback_render)(void* ptr, void* obj, EcHttpRequest*, EcDevStream, EcLogger);
*/
 

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT void echttp_init (void);

__LIB_EXPORT void echttp_done (void);

__LIB_EXPORT EcHttpRequest echttp_request_create (const EcString docroot, const EcString tmproot, int header);

__LIB_EXPORT void echttp_request_destroy (EcHttpRequest*);

__LIB_EXPORT void echttp_request_process (EcHttpRequest, EcSocket);

__LIB_EXPORT void echttp_request_process_dev (EcHttpRequest, EcDevStream, void* callback_ptr);

__LIB_EXPORT void echttp_request_callbacks (EcHttpRequest, EcHttpCallbacks*);

__LIB_EXPORT void echttp_parse_cookies (EcHttpHeader*, const EcString s);

__LIB_EXPORT EcUdc echttp_parse_auth (const EcString source);

// misc

__LIB_EXPORT void echttp_unescape (EcString url);

__LIB_EXPORT void echttp_escape (EcDevStream stream, const EcString url);

__LIB_EXPORT void echttp_escape_stream (EcStream stream, const EcString url);

__LIB_EXPORT const EcString echttp_url_lastPart (const EcString url);

__LIB_EXPORT void echttp_url (EcHttpHeader* header, EcDevStream stream, const EcString url);

__LIB_EXPORT void echttp_realurl (EcHttpHeader* header, EcDevStream stream, const EcString url);

__LIB_EXPORT void echttp_send_header (EcHttpHeader* header, EcDevStream stream, const EcString code, EcUdc extra_params);

__LIB_EXPORT void echttp_send_ErrHeader (EcHttpHeader* header, EcDevStream stream, ulong_t errcode, EcUdc extra_params);

__LIB_EXPORT void echttp_send_DefaultHeader (EcHttpHeader* header, EcDevStream stream, EcUdc extra_params);

__LIB_EXPORT const EcString echttp_getMimeType (const EcString filename);

__CPP_EXTERN______________________________________________________________________________END

#endif
