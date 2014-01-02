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

#include "eclocale.h"
#include "../types/ecmap.h"

#include "../system/ecfile.h"
#include "../utils/ecobserver.h"

#include "ecxmlstream.h"
#include <string.h>

typedef struct
{
  
  EcFileObserver observer;

  EcString items[400];
  
  uint_t maxitems;
  
  EcLogger logger;

} EcLocaleSetData;

struct EcLocaleSet_s
{

  EcString lang;
  
  // having two data sets
  
  EcLocaleSetData data_usr;
  
  EcLocaleSetData data_sys;  
  
};

/*------------------------------------------------------------------------*/

void eclocaleset_clear_data(EcLocaleSetData* data)
{
  uint_t i;
  
  for (i = 0; i < data->maxitems; i++)
  {
    ecstr_delete(&(data->items[i]));
  }
  
  data->maxitems = 0;  
}

/*------------------------------------------------------------------------*/

void eclocaleset_delete_data(EcLocaleSetData* data)
{
  eclocaleset_clear_data(data);
  ecf_observer_delete(data->observer);
}

/*------------------------------------------------------------------------*/

const EcString eclocaleset_get_data(EcLocaleSetData* data, uint_t index)
{
  if((index > 0) && (index < data->maxitems))
  {
    return data->items[index - 1];                
  }  
  return 0;
}

/*------------------------------------------------------------------------*/

void eclocaleset_clear(EcLocaleSet self)
{
  eclocaleset_clear_data(&(self->data_sys));
  eclocaleset_clear_data(&(self->data_usr));
}

/*------------------------------------------------------------------------*/

void eclocaleset_delete(EcLocaleSet* ptr)
{
  EcLocaleSet self = *ptr;
    
  eclocaleset_delete_data(&(self->data_sys));
  eclocaleset_delete_data(&(self->data_usr));
  
  ecstr_delete(&(self->lang));
  
  ENTC_DEL(ptr, struct EcLocaleSet_s);
}

/*------------------------------------------------------------------------*/

const EcString eclocaleset_get(EcLocaleSet self, const EcString text)
{
  if (self == NULL)
  {
    return text;
  }
  
  if (ecstr_empty(text))
  {
    return text;
  }
  
  if (text[0] != 0 && text[0] == '#') 
  {
    if (text[1] != 0 && text[1] == 'S')
    {
      const EcString ret = eclocaleset_get_data(&(self->data_sys), atoi(text + 2));
      if (ecstr_valid(ret))
      {
        return ret;
      }
    }
    else if (text[1] != 0 && text[1] == 'U')
    {
      const EcString ret = eclocaleset_get_data(&(self->data_usr), atoi(text + 2));
      if (ecstr_valid(ret))
      {
        return ret;
      }
    }
  }
  return text;
}

/*------------------------------------------------------------------------*/

void eclocale_parseXml(EcLocaleSetData* data)
{
  EcXMLStream xmlstream = ecxmlstream_openobserver(data->observer, data->logger);
  
  while( ecxmlstream_nextNode( xmlstream ) )
  {
    if( ecxmlstream_isBegin( xmlstream, "q4locale" ) )
    {
      //ecstr_replace(&(self->desc), ecxmlstream_nodeAttribute( xmlstream, "lang" ));
      
      ENTC_XMLSTREAM_BEGIN
      
      if( ecxmlstream_isBegin( xmlstream, "item" ) )
      {
        data->items[data->maxitems] = ecstr_init();
        ecxmlstream_parseNodeValue( xmlstream, &(data->items[data->maxitems]), "item");
        data->maxitems++;
      }
      
      ENTC_XMLSTREAM_END( "q4locale" )
    }
  }
    
  ecxmlstream_close(xmlstream);  
}

/*------------------------------------------------------------------------*/

void eclocale_onchange(void* ptr)
{
  EcLocaleSetData* data = ptr;
  
  eclocaleset_clear_data(data);
  
  eclocale_parseXml(data);
}

/*------------------------------------------------------------------------*/

void eclocale_parseFile_data(EcLocaleSetData* data, const EcString path, const EcString filename, EcEventFiles events , EcLogger logger)
{
  data->observer = ecf_observer_newFromPath(path, filename, path, events, logger, eclocale_onchange, data);
  
  data->maxitems = 0;
  data->logger = logger;
  
  eclocale_parseXml(data);
}

/*------------------------------------------------------------------------*/

void eclocale_parseFile_pre(EcLocaleSet self, const EcString path, const EcString filename, EcEventFiles events , EcLogger logger, const EcString langcode)
{
  if (ecstr_equal(langcode, "sys"))
  {
    eclocale_parseFile_data(&(self->data_sys), path, filename, events, logger);
  }
  else if (ecstr_equal(langcode, "user"))
  {
    eclocale_parseFile_data(&(self->data_usr), path, filename, events, logger);    
  }
}

/*------------------------------------------------------------------------*/

void eclocale_parseFile(EcLocale self, const EcString path, const EcString filename, EcEventFiles events , EcLogger logger)
{
  // remove file extension
  EcString langcode = ecfs_extractFileName(filename);
  // search for the occurance of '_'
  EcString pos = strrchr( langcode, '_' );
  if (pos)
  {
	EcMapNode node;
    EcString lang = ecstr_copy(pos + 1);
    *pos = 0;
    
    node = ecmap_find(self->languages, lang);
    if (node == ecmap_end(self->languages))
    {
      EcLocaleSet lset = ENTC_NEW(struct EcLocaleSet_s);
      lset->lang = lang;

      eclocale_parseFile_pre(lset, path, filename, events, logger, langcode);

      ecmap_append(self->languages, lset->lang, lset);

      eclogger_logformat(logger, LL_DEBUG, "CORE", "{locale} Successful registered '%s'", lset->lang);    
    }
    else
    {
      eclocale_parseFile_pre(ecmap_data(node), path, filename, events, logger, langcode);
      
      ecstr_delete(&lang);
    }

  }
  ecstr_delete(&langcode);
}

/*------------------------------------------------------------------------*/

EcLocale eclocale_new(const EcString confdir, const EcString path, EcEventFiles events , EcLogger logger)
{
  EcLocale self = ENTC_NEW(struct EcLocale_s);
  
  EcString fullpath = ecfs_mergeToPath(confdir, path);

  EcDirHandle dh = ecdh_new(fullpath);
  
  self->languages = ecmap_new();
  
  if (dh)
  {
    EcDirNode dn;
    while (ecdh_next(dh, &dn))
    {
      int filetype = ecdh_getFileType(fullpath, dn);
      
      if( filetype == ENTC_FILETYPE_ISFILE )
      {
        eclocale_parseFile(self, fullpath, dn->d_name, events, logger);
      }
    }
    ecdh_close(&dh);
  }
  
  ecstr_delete(&fullpath);
  
  return self;
}

/*------------------------------------------------------------------------*/

void eclocale_delete(EcLocale* ptr)
{  
  EcLocale self = *ptr;
  
  EcMapNode node;
  
  for (node = ecmap_first(self->languages); node != ecmap_end(self->languages); node = ecmap_next(node)) 
  {
    EcLocaleSet lset = ecmap_data(node);
    
    eclocaleset_delete(&lset);
  }
   
  ecmap_delete(&(self->languages));
  
  ENTC_DEL(ptr, struct EcLocale_s);
}

/*------------------------------------------------------------------------*/

EcLocaleSet eclocale_getSet(EcLocale self, const EcString lang)
{
  EcMapNode node;
  
  node = ecmap_find(self->languages, lang);
  if( node != ecmap_end(self->languages))
  {
    return ecmap_data(node);
  }
  return NULL;
}

/*------------------------------------------------------------------------*/

EcLocaleSet eclocale_getDefaultSet(EcLocale self)
{
  EcMapNode node;

  node = ecmap_first(self->languages);
  if( node != ecmap_end(self->languages))
  {
    return ecmap_data(node);    
  }
  
  return NULL;
}

/*------------------------------------------------------------------------*/

const EcString eclocale_lang(EcLocaleSet self)
{
  return self->lang;
}

/*------------------------------------------------------------------------*/

const EcString eclocale_desc(EcLocaleSet self)
{
  return eclocaleset_get(self, "#S1");
}

/*------------------------------------------------------------------------*/
