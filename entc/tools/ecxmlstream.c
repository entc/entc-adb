/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "ecxmlstream.h"

#include "../utils/ecreadbuffer.h"
#include "../system/ecfile.h"
#include "../utils/ecobserver.h"
#include "../utils/ecsecfile.h"

#include "../types/ecstream.h"
#include "../types/ecmapchar.h"
#include "../types/ecstack.h"
#include "types/ecmap.h"

#include <string.h>
#include <fcntl.h>

/*------------------------------------------------------------------------*/

struct EcXMLStream_s
{
  
  /* sources */
  EcReadBuffer readbuffer;
  
  const char* constbuffer;

  EcFileObserver observer;
  
  /* misc */
  EcStack nodes;
  
  EcMapChar lastattrs;
  
  EcString value;
  
  EcString filename;
  
  ubyte_t lastype;
  
  EcMap namespaces;
  
};


typedef struct 
{
  
  EcString name;
  
  EcString namespace;
  
} EcXMLTag_s; typedef EcXMLTag_s* EcXMLTag;

/*------------------------------------------------------------------------*/

EcXMLStream ecxmlstream_new (void)
{
  EcXMLStream self = ENTC_NEW(struct EcXMLStream_s);
  /* fhandle */
  self->observer = 0;
  /* data sources */
  self->readbuffer = 0;
  self->constbuffer = 0;

  self->nodes = ecstack_new();
  self->lastattrs = ecmapchar_create (EC_ALLOC);
  self->lastype = ENTC_XMLTYPE_NONE;
  self->value = 0; 
  self->filename = 0;
  
  self->namespaces = ecmap_create (EC_ALLOC);
  
  return self;
}

/*------------------------------------------------------------------------*/

EcXMLStream ecxmlstream_openfile (const char* filename, const char* confdir)
{
  /* create the instance */
  EcXMLStream self = ecxmlstream_new ();
  /* create a new readbuffer for the file */
  struct EcSecFopen st;
  
  if (ecsec_fopen(&st, filename, O_RDONLY, confdir))
  {
    /* transfer ownership to our object */
    self->readbuffer = ecreadbuffer_create (st.fhandle, TRUE);
  }
  /* clean up */
  self->filename = st.filename;
  
  return self;
}

/*------------------------------------------------------------------------*/

EcXMLStream ecxmlstream_openpath (const char* path, const char* filename, const char* confdir)
{
  char* file = ecfs_mergeToPath(path, filename);
  
  EcXMLStream self = ecxmlstream_openfile(file, confdir);
  
  free( file );
  
  return self;
}

/*------------------------------------------------------------------------*/

EcXMLStream ecxmlstream_openobserver (EcFileObserver observer)
{
  EcXMLStream self = ecxmlstream_new ();
  /* create a new readbuffer for the file */
  EcFileHandle fhandle = ecf_observer_open(observer);

  if( fhandle )
  {
    self->readbuffer = ecreadbuffer_create (fhandle, FALSE);

    self->observer = observer;
    
  //eclogger_logformat(self->logger, LOGMSG_XML, "CORE", "xmlstream open file '%s' with file observer", ecfile_getFileName(observer));
  }
  
  return self;
}

/*------------------------------------------------------------------------*/

EcXMLStream ecxmlstream_openbuffer (const char* buffer)
{
  EcXMLStream self = ecxmlstream_new ();

  self->constbuffer = buffer;
  
  //eclogger_msg (LL_TRACE, "ENTC", "xml", buffer);

  return self;
}

/*------------------------------------------------------------------------*/

void ecxmlstream_clean (EcXMLStream self)
{
  /* delete all remaining node names */
  EcXMLTag tag = ecstack_top (self->nodes);

  while (isAssigned (tag))
  {
    // clean the tag
    ecstr_delete (&(tag->name));
    ecstr_delete (&(tag->namespace));
    
    ENTC_DEL (&tag, EcXMLTag_s);    
    /* get the element */    
    ecstack_pop (self->nodes);
    /* get next char* */
    tag = ecstack_top (self->nodes);
  }

  {
    EcMapNode node;
    for (node = ecmap_first (self->namespaces); node != ecmap_end (self->namespaces); node = ecmap_next (node))
    {
      EcString val = ecmap_data (node);
      ecstr_delete (&val);
    }  
  }
}

/*------------------------------------------------------------------------*/

void ecxmlstream_close( EcXMLStream self )
{
  ecxmlstream_clean( self );
  /* signal the observer that the file handle can be closed */
  if( self->observer )
  {
    ecf_observer_close( self->observer );
  }
  /* delete the readbuffer */
  if( self->readbuffer )
  {
    ecreadbuffer_destroy (&(self->readbuffer));
  }

  ecstack_delete( &(self->nodes) );

  ecmapchar_destroy (EC_ALLOC, &(self->lastattrs));
  
  ecstr_delete( &(self->value) );
  
  ecstr_delete( &(self->filename) );
  
  ecmap_destroy (EC_ALLOC, &(self->namespaces));

  ENTC_DEL(&self, struct EcXMLStream_s);
}

//-----------------------------------------------------------------------------------------------------------

void ecxmlstream_find_namespace (EcXMLStream self, EcString* tagname, EcString* nsname)
{
  EcString ns1 = NULL;
  EcString ns2 = NULL;
  
  if (ecstr_split (*tagname, &ns1, &ns2, ':'))
  {
    EcMapNode nsnode = ecmap_find (self->namespaces, ns1);
    if (nsnode != ecmap_end (self->namespaces))
    {
      //eclogger_fmt(LL_TRACE, "ENTC", "xml", "found namespace '%s' in tag '%s'", ns1, ns2);

      ecstr_replaceTO (tagname, ns2);
      ecstr_replaceTO (nsname, ns1);

      return;
    }
  }
  
  ecstr_delete(&ns1);
  ecstr_delete(&ns2);
}

//-----------------------------------------------------------------------------------------------------------

const EcString ecxmlstream_getNamespace (EcXMLStream self, const EcString namespace)
{
  EcMapNode node;
  for (node = ecmap_first (self->namespaces); node != ecmap_end (self->namespaces); node = ecmap_next (node))
  {
    if (ecstr_equal (namespace, ecmap_data (node)))
    {
      return ecmap_key (node);
    }
  }
  return NULL;
}

//-----------------------------------------------------------------------------------------------------------

void ecxmlstream_mapNamespaces (EcXMLStream self, EcMapChar nsmap)
{
  EcMapNode node;
  for (node = ecmap_first (self->namespaces); node != ecmap_end (self->namespaces); node = ecmap_next (node))
  {
    ecmapchar_append (nsmap, ecmap_data (node), ecmap_key (node));
  }
}

//-----------------------------------------------------------------------------------------------------------

EcXMLTag ecxmlstream_create_tag (EcXMLStream self, EcString tagname)
{
  EcXMLTag tag = ENTC_NEW (EcXMLTag_s);

  tag->name = tagname;
  tag->namespace = ecstr_init ();    
  
  ecxmlstream_find_namespace (self, &(tag->name), &(tag->namespace));

  return tag;
}

//-----------------------------------------------------------------------------------------------------------

void ecxmlstream_parseNode (EcXMLStream self, const char* node)
{
  //eclogger_logformat(self->logger, LOGMSG_XML, "parse node '%s'", node);
  EcString tagname = 0;
  
  const char* pos = node;
  const char* key_pos = 0;
  EcString key_val = 0;
  const char* value_pos = 0;
  /* get first the node name */
  while( *pos )
  {
    if( *pos == ' ' )
      break;
    if( *pos == '/' )
      break;
    
    pos++;
  }
  
  ecstr_replacePos (&tagname, node, pos);

  //eclogger_logformat(self->logger, LOGMSG_XML, "CORE", "found node '%s' from '%s'", selfnode, node);

  ecmapchar_clear(self->lastattrs);
  
  //eclogger_logformat(self->logger, LOGMSG_XML, "found node '%s'", selfnode);
  /* try to find all attributes */
  while( *pos )
  {
    if( key_pos )
    {
      if( *pos == '=' )
      {
        /* end of key, copy to the temporary char */
        ecstr_replacePos(&key_val, key_pos, pos);
        /* trim spaces from ends */
        ecstr_replaceTO(&key_val, ecstr_trim(key_val));
        /* reset the key position */
        key_pos = 0;
        value_pos = 0;
      }
    }
    else if( value_pos )
    {
      if( *pos == '"' )
      {
        /* add a new key */
        EcString value = ecstr_init();
        
        ecstr_replacePos(&value, value_pos + 1, pos);
        
        if (ecstr_leading (key_val, "xmlns:"))
        {
          EcString ns1 = NULL;
          EcString ns2 = NULL;
          
          if (ecstr_split (key_val, &ns1, &ns2, ':'))
          {
            ecmap_append (self->namespaces, ns2, value);
          }
          else
          {
            ecstr_delete( &value ); 
          }

          ecstr_delete (&ns1);
          ecstr_delete (&ns2);
        }        
        else
        {
          ecmapchar_append(self->lastattrs, key_val, value);
          ecstr_delete( &value );
        }
        
        /*
        eclogger_logformat(self->logger, LOGMSG_XML, "QXML", "found attribute [%s]=[%s]", key_val, value );
        */
        ecstr_delete( &key_val );
        value_pos = 0;
        
      }      
    }
    else if( key_val )
    {
      if( *pos == '"' )
        value_pos = pos;
    }
    else
    {
      /* start with the key, applying filter */
      if( *pos != ' ' )
        key_pos = pos;
    }
    pos++;  
  }
  // clean up
  ecstr_delete( &key_val );
    
  ecstack_push (self->nodes, ecxmlstream_create_tag (self, tagname));
}

/*------------------------------------------------------------------------*/

void ecxmlstream_cleanLastNode( EcXMLStream self )
{
  if( self->lastype == ENTC_XMLTYPE_SINGLE || self->lastype == ENTC_XMLTYPE_NEND )
  {
    /* remove the top element */
    EcXMLTag tag = ecstack_top (self->nodes);

    if (isAssigned (tag))
    {
      ecstr_delete (&(tag->name));
      ecstr_delete (&(tag->namespace));

      ENTC_DEL (&tag, EcXMLTag_s);
      
	    ecstack_pop (self->nodes);      
    }
    else
    {
      eclogger_msg (LL_TRACE, "ENTC", "xml", "can't clean up last node");
    }
  }
  else if( self->lastype == ENTC_XMLTYPE_VALUE )
  {
    ecstr_delete( &(self->value) );
  }
  
  self->lastype = ENTC_XMLTYPE_NONE;
}

/*------------------------------------------------------------------------*/

/*
 return 0 if no chars are available
 */
char ecxmlstream_nextChar( EcXMLStream self )
{
  char c;

  if( self->readbuffer )
  {
    if( ecreadbuffer_getnext( self->readbuffer, &c ) )
    {
      return c;  
    }
  }
  else if( self->constbuffer )
  {
    /* copy the first char from the buffer */
    c = *(self->constbuffer);
    /* increase the buffer position */
    self->constbuffer++;
    
    return c;
  }

  return 0;
}

/*------------------------------------------------------------------------*/

uint_t ecxmlstream_nextGet( EcXMLStream self, uint_t size )
{
  if( self->readbuffer )
  {
    return ecreadbuffer_get( self->readbuffer, size );
  }
  else if( self->constbuffer )
  {
    return strlen( self->constbuffer );
  }
  
  return 0;
}

/*------------------------------------------------------------------------*/

const EcString ecxmlstream_nextBuffer( EcXMLStream self )
{
  if( self->readbuffer )
  {
    return ecreadbuffer_buffer( self->readbuffer );
  }
  else if( self->constbuffer )
  {
    return self->constbuffer;
  }
  
  return 0;
}

/*------------------------------------------------------------------------*/

const EcString ecxmlstream_nodeName (EcXMLStream self)
{
  EcXMLTag tag = ecstack_top (self->nodes);
  
  if (isAssigned (tag))
  {
    return tag->name;
  }
  
  return NULL;
}

/*------------------------------------------------------------------------*/

const EcString ecxmlstream_nodeNamespace (EcXMLStream self)
{
  EcXMLTag tag = ecstack_top (self->nodes);
  
  if (isAssigned (tag))
  {
    return tag->namespace;
  }
  
  return NULL;  
}

/*------------------------------------------------------------------------*/

ubyte_t ecxmlstream_nodeType( EcXMLStream self )
{
  return self->lastype;
}

/*------------------------------------------------------------------------*/

const char* ecxmlstream_nodeAttribute( EcXMLStream self, const char* name )
{
  return ecmapchar_finddata( self->lastattrs, name );
}

/*------------------------------------------------------------------------*/

const char* ecxmlstream_nodeValue( EcXMLStream self )
{
  return self->value;
}

/*------------------------------------------------------------------------*/

const EcString ecxmlstream_isNode( EcXMLStream self )
{
  if ((self->lastype == ENTC_XMLTYPE_SINGLE)||(self->lastype == ENTC_XMLTYPE_NSTART))
  {
    EcXMLTag tag = ecstack_top (self->nodes);
    
    if (isAssigned (tag))
    {
      return tag->name;
    }    
  }

  return NULL;  
}

/*------------------------------------------------------------------------*/

int ecxmlstream_isBegin (EcXMLStream self, const char* name)
{
  if ((self->lastype == ENTC_XMLTYPE_SINGLE)||(self->lastype == ENTC_XMLTYPE_NSTART))
  {
    EcXMLTag tag = ecstack_top (self->nodes);
    
    return isAssigned (tag) && ecstr_equal(tag->name, name);
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

int ecxmlstream_isEnd( EcXMLStream self, const char* name )
{
  if (self->lastype == ENTC_XMLTYPE_NEND)
  {
    EcXMLTag tag = ecstack_top (self->nodes);
    
    return isAssigned (tag) && ecstr_equal(tag->name, name);    
  }
  return FALSE;
}

/*------------------------------------------------------------------------*/

int ecxmlstream_isOpen (EcXMLStream self)
{
  if( self )
  {
    return self->lastype == ENTC_XMLTYPE_NSTART;    
  }
  
  return FALSE;
}

/*------------------------------------------------------------------------*/

int ecxmlstream_isValue (EcXMLStream self)
{
  return self->lastype == ENTC_XMLTYPE_VALUE;
}

/*------------------------------------------------------------------------*/

int ecxmlstream_checkValue( EcXMLStream self, EcStream stream_tag )
{
  EcString value = ecstr_trim( ecstream_get (stream_tag) );
  
  ecstream_clear( stream_tag );

  if( !value )
  {
    return FALSE;  
  }
  
  if( value[0] != 0 )
  {
    //eclogger_logformat(self->logger, LOGMSG_XML, "CORE", "found value [%s]", value);
    
    // clean up the last type
    ecxmlstream_cleanLastNode (self);

    self->lastype = ENTC_XMLTYPE_VALUE;

    ecstr_replaceTO( &(self->value), value );
    
    return TRUE;
  }
  
  ecstr_delete( &value );

  return FALSE;
}

/*------------------------------------------------------------------------*/

void ecxmlstream_logError( EcXMLStream self )
{
  if( self->readbuffer )
  {
    if( self->observer )
    {
      eclogger_fmt (LL_ERROR, "ENTC", "xml", "error in parsing '%s'", ecf_observer_getFileName(self->observer));        
    }
    else if( self->filename )
    {
      eclogger_fmt (LL_ERROR, "ENTC", "xml", "error in parsing '%s'", self->filename);      
    }
    else
    {
      eclogger_msg (LL_ERROR, "ENTC", "xml", "error in parsing in unknown file #1");            
    }
  }
  else if( self->constbuffer )
  {
    eclogger_msg (LL_ERROR, "ENTC", "xml", "error in parsing buffer" );  
  }
  else
  {
    eclogger_msg (LL_ERROR, "ENTC", "xml", "error in parsing in unknown file #2");    
  }
}

/*------------------------------------------------------------------------*/

int ecxmlstream_checkNode( EcXMLStream self, EcStream stream_tag )
{
  /* get the current node name */
  const EcString node = ecstream_get ( stream_tag );
  
  uint_t lpos = ecstream_size(stream_tag);
  
  /* clean up last type */
  ecxmlstream_cleanLastNode( self );
  
  
  if( node[lpos - 1] == '/' )
  {
    ecxmlstream_parseNode( self, node );
    
    self->lastype = ENTC_XMLTYPE_SINGLE;
  }
  else if( node[0] == '/')
  {
    EcXMLTag tag = ecstack_top (self->nodes);
    
    if (isNotAssigned (tag))
    {
      eclogger_fmt (LL_ERROR, "ENTC", "xml", "SYNTAX Error: start and end tag missmatch [NULL][%s]", node + 1);

      ecxmlstream_logError (self);
      return FALSE;      
    }

    if (tag->namespace)
    {
      EcString fullTagName = ecstr_catc (tag->namespace, ':', tag->name);

      if (!ecstr_equal (fullTagName, node + 1))
      {
        eclogger_fmt (LL_ERROR, "ENTC", "xml", "SYNTAX Error: start and end tag missmatch [%s][%s]", fullTagName, node + 1);
        
        ecxmlstream_logError (self);
        
        ecstr_delete (&fullTagName);
        
        return FALSE;      
      }
      
      ecstr_delete (&fullTagName);      
    }
    else
    {
      if (!ecstr_equal (tag->name, node + 1))
      {
        eclogger_fmt (LL_ERROR, "ENTC", "xml", "SYNTAX Error: start and end tag missmatch [%s][%s]", tag->name, node + 1);
        
        ecxmlstream_logError (self);
        return FALSE;      
      }
    }
    
    self->lastype = ENTC_XMLTYPE_NEND;
  }
  else
  {
    ecxmlstream_parseNode( self, node );
    /* start the value flag */
    self->lastype = ENTC_XMLTYPE_NSTART;
  }

  ecstream_clear( stream_tag );
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

int ecxmlstream_do1( EcXMLStream self, EcStream stream_tag )
{
  /* try to find node */
  /* <![CDATA[Inhalt]]]]> */
  
  char c = 0;
  
  /* value for the state machine */
  ubyte_t state = 0;
  
  if( self->lastype == ENTC_XMLTYPE_VALUE )
  {
    state = 1;
  }
  
  c = ecxmlstream_nextChar( self );
  
  while( c )
  {
    //eclogger_logformat(self->logger, LOGMSG_XML, "CORE", "'%c'", c);
    
    switch( state )
    {
      /* start mode */
      case 0:
        
        if( c == '<' )
        {          
          /* node start */
          uint_t size = ecxmlstream_nextGet( self, 8 );
          if( size >= 8 )
          {
			//eclogger_logformat (self->logger, LL_TRACE, "CORE", "node: '%s'", ecxmlstream_nextBuffer(self));

            if( ecstr_equaln(ecxmlstream_nextBuffer(self), "?xml", 4) )
            {
              state = 3;
              continue;
            }
            else if( ecstr_equaln(ecxmlstream_nextBuffer(self), "![CDATA[", 8))
            {
              state = 2;
              continue;
            }
          }
          
          if( ecxmlstream_checkValue( self, stream_tag ) )
          {
            return TRUE;
          }
          /* set mode to inner node */
          state = 1;
          
          ecstream_clear( stream_tag );
        }
        else
        {
          /* accumulate all characters for the value */
          ecstream_append_c( stream_tag, c );
        }        

        break;
      /* node mode */
      case 1:

        if( c == '<' )
        {
          /* *** ERROR *** */
          eclogger_fmt (LL_ERROR, "ENTC", "xml", "SYNTAX Error, found '<' inside node '%s'", ecstream_get( stream_tag ));
          
          ecxmlstream_logError(self);

          return FALSE;
        }
        else if( c == '>' )
        {
          if( !ecxmlstream_checkNode( self, stream_tag ) )
          {
            return FALSE;
          }
          
          state = 0;
          return TRUE;
        }
        else
        {
          /* accumulate all characters for the node */
          ecstream_append_c( stream_tag, c );
        }
        
      break;
      /* CDATA mode */
      case 2:
      {
        uint_t size = ecxmlstream_nextGet( self, 3 );
        if( size >= 3 )
        {
          if( ecstr_equaln(ecxmlstream_nextBuffer(self), "]]>", 3))
          {
            state = 0;
            break;
          }
        }
        
        /* accumulate all characters for the value */
        ecstream_append_c( stream_tag, c );
      }
      break;
      /* ?xml */ 
      case 3:
      {
        uint_t size = ecxmlstream_nextGet( self, 2 );
        if( size >= 2 )
        {
          if( ecstr_equaln(ecxmlstream_nextBuffer(self), "?>", 2))
          {
            state = 0;
            break;
          }
        }        
      }
      break;
        
      default:
      break;
    }

    c = ecxmlstream_nextChar( self );
  }

  return c != 0;
}

/*------------------------------------------------------------------------*/

int ecxmlstream_nextNode( EcXMLStream self )
{
  EcStream stream_tag = ecstream_create();
  
  int res = ecxmlstream_do1(self, stream_tag);
  
  ecstream_destroy( &stream_tag );
  
  return res;
}

/*------------------------------------------------------------------------*/

void ecxmlstream_parseNodeValue( EcXMLStream self, EcString* value, const EcString node)
{
  if( ecxmlstream_isOpen( self ) ) while( ecxmlstream_nextNode( self ) )
  {
    if( ecxmlstream_isValue( self ) )
    {
      ecstr_replace(value, ecxmlstream_nodeValue( self ) );          
    }
    else if( ecxmlstream_isEnd( self, node ) )
    {
      break;    
    }  
  }
}

/*------------------------------------------------------------------------*/
