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

#include "adbo_item.h"

#include "adbo_object.h"

struct AdboItem_s
{
  
  int dummy;
  
};

//----------------------------------------------------------------------------------------

AdboItem adbo_item_new1 (AdboObject obj, AdboContainer parent, EcXMLStream xmlstream, const EcString tag, EcLogger logger)
{
  AdboItem self = ENTC_NEW (struct AdboItem_s);
    
  if (isAssigned (xmlstream))
  {
    ENTC_XMLSTREAM_BEGIN
    
    if (ecxmlstream_isBegin (xmlstream, "value"))
    {
      adbo_setValue (obj, adbo_value_newFromXml (xmlstream));
    }
    else if (ecxmlstream_isBegin (xmlstream, "data"))
    {
      
    }
    
    ENTC_XMLSTREAM_END (tag)
  }
  
  return self;  
}

//----------------------------------------------------------------------------------------

AdboItem adbo_item_new2 (AdboObject obj, AdboValue value)
{
  AdboItem self = ENTC_NEW (struct AdboItem_s);

  adbo_setValue (obj, adbo_value_clone (value));
  
  return self;
}

//----------------------------------------------------------------------------------------

AdboItem adbo_item_clone (AdboItem oself)
{
  AdboItem self = ENTC_NEW (struct AdboItem_s);
  
  
  return self;  
}

//----------------------------------------------------------------------------------------

void adbo_item_strToStream (AdboObject obj, AdboItem self, EcStream stream)
{
  AdboValue value = (AdboValue)adbo_getValue (obj);
  if (isAssigned (value))
  {
    const EcString data;
    const EcString dbcolumn = adbo_value_getDBColumn (value);

    ecstream_appendc (stream, '"');

    if (ecstr_valid (dbcolumn))
    {
      ecstream_append (stream, dbcolumn);
    }
    else
    {
      ecstream_append (stream, "#NULL");
    }

    data = adbo_value_get (value, obj);

    ecstream_append (stream, "\":\"");
    ecstream_append (stream, ecstr_cstring(data));
    ecstream_appendc (stream, '"');
  }
}

//----------------------------------------------------------------------------------------

EcString adbo_item_str (AdboObject obj)
{
  AdboValue value = (AdboValue)adbo_getValue (obj);
  if (isAssigned (value))
  {
    return ecstr_copy (adbo_value_get (value, obj));
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

AdboObject adbo_item_get (AdboObject obj, const EcString link)
{
  AdboValue value = (AdboValue)adbo_getValue (obj);
  if (isAssigned (value))
  {
    if (ecstr_equal (adbo_value_getDBColumn (value), link))
    {
      return obj;
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_item_udc (AdboObject obj, AdboItem self)
{
  AdboValue value = (AdboValue)adbo_getValue (obj);
  if (isAssigned (value))
  {
    const EcString dbcolumn = adbo_value_getDBColumn (value);
    if (ecstr_valid(dbcolumn))
    {
      EcUdc item = ecudc_create(ENTC_UDC_STRING, ecstr_cstring(dbcolumn));
      ecudc_setS(item, ecstr_cstring(adbo_value_get (value, obj)));
      
      return item;
    }    
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

void adbo_item_dump (AdboObject obj, AdboContainer container, int depth, int le, EcBuffer b2, EcLogger logger)
{
  AdboValue value;

  unsigned char* buffer = b2->buffer;
  
  uint_t pos = depth * 2;
  
  uint_t state = -1000;
  
  const EcString dbcolumn = "none";
  const EcString dbvalue = "none";
  
  buffer [pos] = le ? '\\' : '[';        
  buffer [pos + 1] = '_';
  buffer [pos + 2] = '_';
  buffer [pos + 3] = 0;
  
  value = (AdboValue)adbo_getValue (obj);
  if (isAssigned (value))
  {
    dbcolumn = adbo_value_getDBColumn (value);
    dbvalue = adbo_value_cget (value, container);
    state = adbo_value_getState (value);
  }
    
  eclogger_logformat(logger, LL_TRACE, "ADBO", "%s ['%s' = '%s'] state: %s"
                       , ecstr_get (b2)
                       , dbcolumn
                       , dbvalue
                       , adbo_dump_state (state)
                       );    
}

//----------------------------------------------------------------------------------------
