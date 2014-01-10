/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:adbo@kalkhof.org]
 *
 * This file is part of adbo framework (Advanced Database Objects)
 *
 * adbo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * adbo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adbo. If not, see <http://www.gnu.org/licenses/>.
 */

#include "adbo_substitute.h"

#include "adbo_container.h"
#include "adbo_object.h"

#include "types/ecmap.h"

struct AdboSubManager_s
{
  
  EcMap subs;
  
};

//----------------------------------------------------------------------------------------

void adbo_subsmgr_parse_substitutes (AdboSubManager self, AdboContext context, EcXMLStream xmlstream)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "substitute"))
  {
    // need to copy because xmlstream will be used further
    EcString name = ecstr_copy (ecxmlstream_nodeAttribute (xmlstream, "name"));
    if (ecstr_valid (name))
    {
      AdboObject obj;
      // obviously we found a substitute
      eclogger_logformat(context->logger, LL_TRACE, "ADBO", "{scan} found substitute '%s'", name );
      
      obj = adbo_object_new (NULL, context, ADBO_OBJECT_SUBSTITUTE, xmlstream, "substitute");
      
      ecmap_append (self->subs, name, obj);
    }
    ecstr_delete(&name);
  }
  
  ENTC_XMLSTREAM_END( "adbo_substitutes" )  
}

//----------------------------------------------------------------------------------------

void adbo_subsmgr_parse (AdboSubManager self, AdboContext context, const EcString filename, const EcString confdir)
{
  EcXMLStream xmlstream = ecxmlstream_openfile(filename, context->logger, confdir);
  
  while( ecxmlstream_nextNode( xmlstream ) )
  {
    if( ecxmlstream_isBegin( xmlstream, "adbo_substitutes" ) )
    {
      adbo_subsmgr_parse_substitutes (self, context, xmlstream);
    }
  }  
  
  ecxmlstream_close (xmlstream);
}

//----------------------------------------------------------------------------------------

void adbo_subsmgr_scan (AdboSubManager self, AdboContext context, const EcString scanpath)
{
  EcListNode node;
  EcList files = eclist_new ();  
  
  eclogger_logformat(context->logger, LL_TRACE, "ADBO", "{scan} scan path '%s' for adbo substitutes", scanpath);        
  // fill a list with all files in that directory
  if (!ecdh_scan(scanpath, files, ENTC_FILETYPE_ISFILE))
  {
    eclogger_logformat(context->logger, LL_ERROR, "ADBO", "{scan} can't find path '%s'", ecstr_cstring(scanpath) );    
  }  
  for (node = eclist_first(files); node != eclist_end(files); node = eclist_next(node))
  {
    EcString filename = eclist_data(node);
    // check xml content
    const EcString extension = ecfs_extractFileExtension(filename);
    if (ecstr_equal(extension, "xml"))
    {
      adbo_subsmgr_parse (self, context, filename, scanpath);
    }
    // clean up
    ecstr_delete(&filename);
  }
  // clean up
  eclist_delete(&files);  
}

//----------------------------------------------------------------------------------------

AdboSubManager adbo_subsmgr_new (const EcString scanpath, AdboContext context)
{
  AdboSubManager self = ENTC_NEW (struct AdboSubManager_s);
  
  context->substitutes = self;
  
  self->subs = ecmap_new ();
    
  adbo_subsmgr_scan (self, context, scanpath);
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_subsmgr_del (AdboSubManager* pself)
{
  AdboSubManager self = *pself;
  
  ecmap_delete (&(self->subs));
  
  ENTC_DEL (pself, struct AdboSubManager_s);
}

//----------------------------------------------------------------------------------------

AdboObject adbo_subsmgr_get (AdboSubManager self, const EcString name)
{
  AdboObject obj;
  
  printf("find '%s'\n", name);
  
  EcMapNode node = ecmap_find(self->subs, name);
  if (node == ecmap_end(self->subs))
  {
    return NULL;
  }
  
  obj = ecmap_data(node);
  
  printf("found obj %p\n", obj);
  
  return obj;
}

//----------------------------------------------------------------------------------------

struct AdboSubstitute_s
{
  
  AdboContainer container;
  
};

//----------------------------------------------------------------------------------------

AdboSubstitute adbo_substitute_new (AdboObject obj, AdboContext context, AdboContainer parent, EcXMLStream xmlstream)
{
  AdboSubstitute self = ENTC_NEW (struct AdboSubstitute_s);

  self->container = adbo_container_new (ADBO_CONTAINER_SUBSTITUTE, parent);

  printf("sub new %p\n", self);
  
  adbo_objects_fromXml (self->container, context, xmlstream, "substitute");
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_substitute_del (AdboSubstitute* pself)
{
  AdboSubstitute self = *pself;
  
  adbo_container_del (&(self->container));
  
  ENTC_DEL (pself, struct AdboSubstitute_s);
}

//----------------------------------------------------------------------------------------

AdboSubstitute adbo_substitute_clone (const AdboSubstitute oself, AdboContainer parent)
{
  AdboSubstitute self = ENTC_NEW (struct AdboSubstitute_s);
  
  printf("clone substitute %p\n", oself);
  
  self->container = adbo_container_clone (oself->container, parent, NULL, NULL, NULL);
  
  return self;
}

//----------------------------------------------------------------------------------------

int adbo_substitute_request (AdboSubstitute self, AdboContext context)
{
  return adbo_container_request (self->container, context);
}

//----------------------------------------------------------------------------------------

int adbo_substitute_update (AdboSubstitute self, AdboContext context, int withTransaction)
{
  return adbo_container_update (self->container, context);
}

//----------------------------------------------------------------------------------------

int adbo_substitute_delete (AdboSubstitute self, AdboContext context, int withTransaction)
{
  return adbo_container_delete (self->container, context);
}

//----------------------------------------------------------------------------------------

void adbo_substitute_transaction (AdboSubstitute self, int state)
{
  adbo_container_transaction (self->container, state);
}

//----------------------------------------------------------------------------------------

AdboObject adbo_substitute_at (AdboSubstitute self, const EcString link)
{
  return adbo_container_at (self->container, link);
}

//----------------------------------------------------------------------------------------

void adbo_substitute_strToStream (AdboSubstitute self, EcStream stream)
{
  adbo_container_str (self->container, stream);
}

//----------------------------------------------------------------------------------------

EcString adbo_substitute_str (AdboSubstitute self)
{
  EcStream stream;
  EcBuffer buffer;
  // recursively continue with the type of this object
  stream = ecstream_new ();
  
  adbo_container_str (self->container, stream);
  // extract the buffer from the stream
  buffer = ecstream_trans (&stream);
  // extract string from buffer
  return ecstr_trans (&buffer);  
}

//----------------------------------------------------------------------------------------

AdboObject adbo_substitute_get (AdboSubstitute self, const EcString link)
{
  return adbo_container_get (self->container, link);
}

//----------------------------------------------------------------------------------------

void adbo_substitute_dump (AdboObject obj, AdboSubstitute self, int tab, int le, EcBuffer b2, EcLogger logger)
{
  adbo_container_dump (self->container, tab, b2, logger);
}

//----------------------------------------------------------------------------------------

void adbo_substitute_addToQuery (AdboSubstitute self, AdblQuery* query)
{
  adbo_container_query (self->container, query);
}

//----------------------------------------------------------------------------------------

void adbo_substitute_setFromQuery (AdboSubstitute self, AdblCursor* cursor, EcLogger logger)
{
  adbo_container_set (self->container, cursor, logger);
}

//----------------------------------------------------------------------------------------

void adbo_substitute_addToAttr (AdboSubstitute self, AdblAttributes* attrs)
{
  adbo_container_attrs (self->container, attrs);
}

//----------------------------------------------------------------------------------------
