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

#include "ecxmldom.h"

#include "types/ecstream.h"

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_add_tag (EcUdc parent, EcUdc child)
{
  EcUdc tags = ecudc_node (parent, ".tags");
  if (isNotAssigned (tags))
  {
    EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, ".tags");
    
    tags = h;
    
    ecudc_add(parent, &h);
  }
  {
    EcUdc h = child;
    
    ecudc_add(tags, &h);
  }
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecxmldom_create_tag (EcUdc parent, const EcString name)
{
  EcUdc tag = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, name);
  
  if (isAssigned (parent))
  {
    ecxmldom_add_tag (parent, tag);
  }
  
  return tag;
}

//-----------------------------------------------------------------------------------------------------------

const EcString ecxmldom_get_name (EcUdc tag)
{
  return ecudc_name(tag);
}

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_set_name (EcUdc tag, const EcString name)
{
  ecudc_setName (tag, name);
}

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_set_value (EcUdc tag, const EcString content)
{
  EcUdc value = ecudc_node (tag, ".value");
  if (isNotAssigned (value))
  {
    ecudc_add_asString (EC_ALLOC, tag, ".value", content);
  } 
  else 
  {
    ecudc_setS (value, content);    
  }
}

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_add_attribute (EcUdc tag, const EcString name, const EcString value)
{
  EcUdc attrs = ecudc_node (tag, ".attrs");
  if (isNotAssigned (attrs))
  {
    EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, ".attrs");
    
    attrs = h;
    
    ecudc_add (tag, &h);
  } 
  // finally add the attribute
  ecudc_add_asString(EC_ALLOC, attrs, name, value);
}

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_write (EcUdc tag, EcStream stream, const EcString namespace)
{
  {
    EcUdc options = ecudc_node (tag, ".options");
    if (isAssigned (options))
    {
      namespace = ecudc_get_asString(options, "namespace", namespace);
    }
  }
  // start
  ecstream_append (stream, "<");
  if (ecstr_valid (namespace)) 
  {
    ecstream_append (stream, namespace);
    ecstream_appendc (stream, ':');
  }
  ecstream_append (stream, ecudc_name (tag));
  
  {
    EcUdc attrs = ecudc_node (tag, ".attrs");
    if (isAssigned (attrs))
    {
      void* cursor = NULL;
      EcUdc item;
      
      // iterate through all tags
      for (item = ecudc_next (attrs, &cursor); isAssigned (item); item = ecudc_next (attrs, &cursor))
      {
        ecstream_appendc (stream, ' ');
        ecstream_append (stream, ecudc_name(item));
        ecstream_append (stream, "=\"");
        ecstream_append (stream, ecudc_asString(item));
        ecstream_appendc (stream, '"');
      }      
    }
  }
  {
    EcUdc tags = ecudc_node (tag, ".tags");
    if (isAssigned (tags))
    {
      ecstream_append (stream, ">\r\n");
      
      {
        void* cursor = NULL;
        EcUdc item;
        
        // iterate through all tags
        for (item = ecudc_next (tags, &cursor); isAssigned (item); item = ecudc_next (tags, &cursor))
        {
          ecxmldom_write (item, stream, namespace);
        }
      }        
      
      ecstream_append (stream, "</");
      if (ecstr_valid (namespace)) 
      {
        ecstream_append (stream, namespace);
        ecstream_appendc (stream, ':');
      }
      ecstream_append (stream, ecudc_name (tag));
      ecstream_append (stream, ">\r\n");
      
      return;
    }
  }
  {
    EcUdc value = ecudc_node (tag, ".value");
    if (isAssigned (value))
    {
      ecstream_append (stream, ">");
      
      ecstream_append (stream, ecudc_asString(value));

      ecstream_append (stream, "</");
      if (ecstr_valid (namespace)) 
      {
        ecstream_append (stream, namespace);
        ecstream_appendc (stream, ':');
      }
      ecstream_append (stream, ecudc_name (tag));
      ecstream_append (stream, ">\r\n");
      
      return;
    }
  }
  
  ecstream_append (stream, "/>\r\n");
}

//-----------------------------------------------------------------------------------------------------------

EcBuffer ecxmldom_buffer (EcUdc* pself)
{
  EcStream stream = ecstream_new ();
  EcUdc tag = *pself;
  
  ecstream_append (stream, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");  
  
  ecxmldom_write (tag, stream, NULL);
  
  ecudc_destroy (EC_ALLOC, pself);
  
  return ecstream_trans (&stream);
}

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_addNamespace (EcUdc tag, const EcString name, const EcString definition)
{
  if (ecstr_valid (definition)) 
  {
    EcString xmlns = ecstr_cat2("xmlns:", name);
    
    ecxmldom_add_attribute (tag, xmlns, definition);
    
    ecstr_delete(&xmlns);
  }  
}

//-----------------------------------------------------------------------------------------------------------

void ecxmldom_setNamespace (EcUdc tag, const EcString name, const EcString definition)
{
  EcUdc options = ecudc_node (tag, ".options");
  if (isNotAssigned (options))
  {
    EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, ".options");
    
    options = h;
    
    ecudc_add (tag, &h);
  } 
  {
    EcUdc opt = ecudc_node(options, "namespace");
    if (isAssigned (opt))
    {
      ecudc_setS(opt, name);
    }
    else
    {
      ecudc_add_asString(EC_ALLOC, options, "namespace", name);
    }
  }
  
  ecxmldom_addNamespace (tag, name, definition);  
}

//-----------------------------------------------------------------------------------------------------------
