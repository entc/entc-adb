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

#ifndef ENTC_TOOLS_XMLSTREAM_H
#define ENTC_TOOLS_XMLSTREAM_H 1

#include "system/macros.h"
#include "system/types.h"
#include "types/ecstring.h"
#include "types/ecmap.h"
#include "tools/eclog.h"

#define ENTC_XMLTYPE_NONE    0
#define ENTC_XMLTYPE_SINGLE  1
#define ENTC_XMLTYPE_NSTART  2
#define ENTC_XMLTYPE_NEND    3
#define ENTC_XMLTYPE_VALUE   4
#define ENTC_XMLTYPE_CDATA   5

struct EcXMLStream_s; typedef struct EcXMLStream_s* EcXMLStream;

__CPP_EXTERN______________________________________________________________________________START
    
__LIB_EXPORT EcXMLStream ecxmlstream_openfile (const char* filename, const char* confdir);

__LIB_EXPORT EcXMLStream ecxmlstream_openpath (const char* path, const char* filename, const char* confdir);

__LIB_EXPORT EcXMLStream ecxmlstream_openbuffer (const char* buffer);

__LIB_EXPORT void ecxmlstream_close (EcXMLStream);
  
__LIB_EXPORT int ecxmlstream_nextNode (EcXMLStream);
  
__LIB_EXPORT const EcString ecxmlstream_nodeName (EcXMLStream);

__LIB_EXPORT const EcString ecxmlstream_nodeNamespace (EcXMLStream);

__LIB_EXPORT ubyte_t ecxmlstream_nodeType (EcXMLStream);
  
__LIB_EXPORT const char* ecxmlstream_nodeAttribute (EcXMLStream, const char* name);

__LIB_EXPORT const char* ecxmlstream_nodeValue (EcXMLStream);
  
__LIB_EXPORT const EcString ecxmlstream_isNode (EcXMLStream);

__LIB_EXPORT int ecxmlstream_isBegin (EcXMLStream, const char* name);

__LIB_EXPORT int ecxmlstream_isEnd (EcXMLStream, const char* name);

__LIB_EXPORT int ecxmlstream_isOpen (EcXMLStream);
  
__LIB_EXPORT int ecxmlstream_isValue (EcXMLStream);
  
  /* helper functions */
  
__LIB_EXPORT void ecxmlstream_parseNodeValue (EcXMLStream, EcString* value, const EcString node);

__LIB_EXPORT const EcString ecxmlstream_getNamespace (EcXMLStream, const EcString);

__LIB_EXPORT void ecxmlstream_mapNamespaces (EcXMLStream, EcMap);
  
__CPP_EXTERN______________________________________________________________________________END

#define ENTC_XMLSTREAM_BEGIN if( ecxmlstream_isOpen( xmlstream ) ) while( ecxmlstream_nextNode( xmlstream ) ) {
#define ENTC_XMLSTREAM_END( endtag ) else if( ecxmlstream_isEnd( xmlstream, endtag ) ) break; }
#define ENTC_XMLSTREAM_EMPTY( endtag ) if( ecxmlstream_isOpen( xmlstream ) ) while( ecxmlstream_nextNode( xmlstream ) ) { if( ecxmlstream_isEnd( xmlstream, endtag ) ) break; }
#define ENTC_XMLSTREAM_CLOSE }

#endif
