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



typedef struct {

  int header_on;
  
  int method;
    
  EcString host;
  
  const EcString remote_address;
  
  const EcString mime;
  
  const EcString url;
  
  EcString user_lang;
  
  EcString user_agent;
  
  EcString sessionid;

  EcString session_lang;
  
  EcString request_params;
  
  EcString request_url;
  
  EcString title;
  
  EcList tokens;
  
  EcListNode token;
  
  EcString urlpath;
  
} EcHttpHeader;

struct EcHttpRequest_s; typedef struct EcHttpRequest_s* EcHttpRequest;

typedef int (_STDCALL *http_process_fct)(void* ptr, EcHttpHeader*, void** object);
typedef int (_STDCALL *http_render_fct)(void* ptr, EcHttpHeader*, EcDevStream, void** object);
typedef int (_STDCALL *http_header_fct)(void* ptr, EcHttpHeader*);

typedef struct {
  
  http_process_fct process;
  
  void* process_ptr;
  
  http_render_fct render;
  
  void* render_ptr;  
  
} EcHttpCallbacks;

/*
typedef int (*http_callback_init)(void* ptr, void** pobj, EcHttpRequest*);
typedef int (*http_callback_done)(void* ptr, void** pobj, EcHttpRequest*);
typedef int (*http_callback_render)(void* ptr, void* obj, EcHttpRequest*, EcDevStream, EcLogger);
*/
 

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT void echttp_init (void);

__LIB_EXPORT void echttp_done (void);

__LIB_EXPORT EcHttpRequest echttp_request_create (const EcString rootdoc, int header, EcLogger);

__LIB_EXPORT void echttp_request_destroy (EcHttpRequest*);

__LIB_EXPORT void echttp_request_process (EcHttpRequest, EcSocket socket, EcLogger logger);

__LIB_EXPORT void echttp_request_process_dev (EcHttpRequest, EcDevStream, http_header_fct, void*, EcLogger logger);

__LIB_EXPORT void echttp_request_callbacks (EcHttpRequest, EcHttpCallbacks*);

__LIB_EXPORT void echttp_parse_cookies (EcHttpHeader*, const EcString s);

// misc

__LIB_EXPORT void echttp_unescape (EcString url);

__LIB_EXPORT void echttp_escape (EcDevStream stream, const EcString url);

__LIB_EXPORT void echttp_url (EcHttpHeader* header, EcDevStream stream, const EcString url);

__LIB_EXPORT void echttp_realurl (EcHttpHeader* header, EcDevStream stream, const EcString url);

__LIB_EXPORT void echttp_send_DefaultHeader (EcHttpHeader* header, EcDevStream stream);

__CPP_EXTERN______________________________________________________________________________END

#endif
