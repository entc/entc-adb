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

#include "sys/entc_export.h"
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

__ENTC_LIBEX EcXMLStream ecxmlstream_openfile (const char* filename, const char* confdir);

__ENTC_LIBEX EcXMLStream ecxmlstream_openpath (const char* path, const char* filename, const char* confdir);

__ENTC_LIBEX EcXMLStream ecxmlstream_openbuffer (const char* buffer);

__ENTC_LIBEX void ecxmlstream_destroy (EcXMLStream*);
  
__ENTC_LIBEX int ecxmlstream_nextNode (EcXMLStream);
  
__ENTC_LIBEX const EcString ecxmlstream_nodeName (EcXMLStream);

__ENTC_LIBEX const EcString ecxmlstream_nodeNamespace (EcXMLStream);

__ENTC_LIBEX ubyte_t ecxmlstream_nodeType (EcXMLStream);
  
__ENTC_LIBEX const char* ecxmlstream_nodeAttribute (EcXMLStream, const char* name);

__ENTC_LIBEX const char* ecxmlstream_nodeValue (EcXMLStream);
  
__ENTC_LIBEX const EcString ecxmlstream_isNode (EcXMLStream);

__ENTC_LIBEX int ecxmlstream_isBegin (EcXMLStream, const char* name);

__ENTC_LIBEX int ecxmlstream_isEnd (EcXMLStream, const char* name);

__ENTC_LIBEX int ecxmlstream_isOpen (EcXMLStream);
  
__ENTC_LIBEX int ecxmlstream_isValue (EcXMLStream);
  
  /* helper functions */
  
__ENTC_LIBEX void ecxmlstream_parseNodeValue (EcXMLStream, EcString* value, const EcString node);

__ENTC_LIBEX const EcString ecxmlstream_getNamespace (EcXMLStream, const EcString);

__ENTC_LIBEX void ecxmlstream_mapNamespaces (EcXMLStream, EcMap);
  
#define ENTC_XMLSTREAM_BEGIN if( ecxmlstream_isOpen( xmlstream ) ) while( ecxmlstream_nextNode( xmlstream ) ) {
#define ENTC_XMLSTREAM_END( endtag ) else if( ecxmlstream_isEnd( xmlstream, endtag ) ) break; }
#define ENTC_XMLSTREAM_EMPTY( endtag ) if( ecxmlstream_isOpen( xmlstream ) ) while( ecxmlstream_nextNode( xmlstream ) ) { if( ecxmlstream_isEnd( xmlstream, endtag ) ) break; }
#define ENTC_XMLSTREAM_CLOSE }

#endif
