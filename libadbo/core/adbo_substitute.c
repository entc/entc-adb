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

#include "types/ecmap.h"

struct AdboSubManager_s
{
  
  EcMap subs;
  
};

//----------------------------------------------------------------------------------------

void adbo_subsmgr_parse_substitutes (AdboSubManager self, EcXMLStream xmlstream, EcLogger logger)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "substitute"))
  {
    // obviously we found a substitute
    eclogger_logformat(logger, LL_TRACE, "ADBO", "{scan} found substitute" );    
  }
  
  ENTC_XMLSTREAM_END( "adbo_substitutes" )  
}

//----------------------------------------------------------------------------------------

void adbo_subsmgr_parse (AdboSubManager self, const EcString filename, EcLogger logger, const EcString confdir)
{
  EcXMLStream xmlstream = ecxmlstream_openfile(filename, logger, confdir);
  
  while( ecxmlstream_nextNode( xmlstream ) )
  {
    if( ecxmlstream_isBegin( xmlstream, "adbo_substitutes" ) )
    {
      adbo_subsmgr_parse_substitutes (self, xmlstream, logger);
    }
  }  
  
  ecxmlstream_close (xmlstream);
}

//----------------------------------------------------------------------------------------

void adbo_subsmgr_scan (AdboSubManager self, const EcString scanpath, EcLogger logger)
{
  EcListNode node;
  EcList files = eclist_new ();  
  
  eclogger_logformat(logger, LL_TRACE, "ADBO", "{scan} scan path '%s' for adbo substitutes", scanpath);        
  // fill a list with all files in that directory
  if (!ecdh_scan(scanpath, files, ENTC_FILETYPE_ISFILE))
  {
    eclogger_logformat(logger, LL_ERROR, "ADBO", "{scan} can't find path '%s'", ecstr_cstring(scanpath) );    
  }  
  for (node = eclist_first(files); node != eclist_end(files); node = eclist_next(node))
  {
    EcString filename = eclist_data(node);
    // check xml content
    const EcString extension = ecfs_extractFileExtension(filename);
    if (ecstr_equal(extension, "xml"))
    {
      adbo_subsmgr_parse (self, filename, logger, scanpath);
    }
    // clean up
    ecstr_delete(&filename);
  }
  // clean up
  eclist_delete(&files);  
}

//----------------------------------------------------------------------------------------

AdboSubManager adbo_subsmgr_new (const EcString scanpath, EcLogger logger)
{
  AdboSubManager self = ENTC_NEW (struct AdboSubManager_s);
  
  self->subs = ecmap_new ();
    
  adbo_subsmgr_scan (self, scanpath, logger);
  
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

struct AdboSubstitute_s
{
  
  AdboContainer container;
  
};

//----------------------------------------------------------------------------------------

AdboSubstitute adbo_substitute_new (AdboObject obj, AdboContainer parent, EcXMLStream xmlstream, EcLogger logger)
{
  AdboSubstitute self = ENTC_NEW (struct AdboSubstitute_s);

  self->container = adbo_container_new (ADBO_CONTAINER_SUBSTITUTE, parent);
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_substitute_del (AdboSubstitute* pself)
{
  
}

//----------------------------------------------------------------------------------------

AdboSubstitute adbo_substitute_clone (const AdboSubstitute oself, AdboContainer parent)
{
  
}

//----------------------------------------------------------------------------------------

int adbo_substitute_request (AdboSubstitute self, AdboContext context)
{
  
}

//----------------------------------------------------------------------------------------

int adbo_substitute_update (AdboSubstitute self, AdboContext context, int withTransaction)
{
  
}

//----------------------------------------------------------------------------------------

int adbo_substitute_delete (AdboSubstitute self, AdboContext context, int withTransaction)
{
  
}

//----------------------------------------------------------------------------------------

void adbo_substitute_transaction (AdboSubstitute self, int state)
{
  
}

//----------------------------------------------------------------------------------------

AdboObject adbo_substitute_at (AdboSubstitute self, const EcString link)
{
  
}

//----------------------------------------------------------------------------------------

void adbo_substitute_strToStream (AdboSubstitute self, EcStream stream)
{
  
}

//----------------------------------------------------------------------------------------

EcString adbo_substitute_str (AdboSubstitute self)
{
  
}

//----------------------------------------------------------------------------------------

int adbo_substitute_is (AdboSubstitute self, const EcString link)
{
  
}

//----------------------------------------------------------------------------------------

void adbo_substitute_dump (AdboObject obj, AdboSubstitute self, int tab, int le, EcBuffer b2, EcLogger logger)
{

}
