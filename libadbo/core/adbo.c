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

#include "adbo.h"
#include "adbo_context_intern.h"
#include <tools/ecdata.h>
#include <tools/ecjson.h>

//----------------------------------------------------------------------------------------

// forward declarations
void adbo_node_fromXml (EcUdc, AdboContext, EcXMLStream);
void adbo_item_fromXml (EcUdc, AdboContext, EcXMLStream, const EcString tag);

//----------------------------------------------------------------------------------------

void adbo_structure_items_fromXml (EcUdc rootNode, AdboContext context, EcXMLStream xmlstream, const EcString tag)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "node"))
  {
    adbo_node_fromXml (rootNode, context, xmlstream);
  }
  else if (ecxmlstream_isBegin (xmlstream, "substitute"))
  {
    
  }
  
  ENTC_XMLSTREAM_END (tag)  
}

//----------------------------------------------------------------------------------------

EcUdc adbo_structure_fromXml (AdboContext context, EcXMLStream xmlstream, const EcString name, const EcString tag)
{
  // create a default data node
  EcUdc node = ecnode_create (name);
  
  // all tables are stored as additional nodes at the first level
  adbo_structure_items_fromXml (node, context, xmlstream, tag);
  
  /*
  {
    EcString json = ecjson_write (node);
    
    printf("%s\n", json);
    
  }
  */
  
  return node;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_structure_fromDatabase (AdboContext context)
{
  return NULL;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_tables (EcUdc node)
{
  return ecudc_node (node, ECDATA_ITEMS);
}

//----------------------------------------------------------------------------------------

void adbo_node_updateSize (EcUdc node, AdboContext context);

void adbo_updateSize (EcUdc rootNode, AdboContext context)
{
  EcUdc items = ecudc_node (rootNode, ECDATA_ITEMS);
  if (isAssigned (items))
  {
    void* cursor = NULL;
    EcUdc item;
  
    for (item  = ecudc_next (items, &cursor); isAssigned (item); item = ecudc_next (items, &cursor))
    {
      adbo_node_updateSize (item, context);
    }
  }
}

//----------------------------------------------------------------------------------------

EcUdc adbo_get_table (EcUdc node, const EcString tablename)
{
  EcUdc items = ecudc_node (node, ECDATA_ITEMS);
  if (isAssigned (items))
  {
    return ecudc_node (items, tablename);
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

int adbo_node_fetch (EcUdc node, EcUdc data, AdboContext context);

int adbo_item_fetch (EcUdc item, EcUdc data, AdboContext context)
{
  return adbo_node_fetch (item, data, context);
}

//----------------------------------------------------------------------------------------

int adbo_node_update (EcUdc udc, EcUdc filter, AdboContext context, EcUdc data, int withTransaction);

int adbo_update (EcUdc udc, EcUdc filter, AdboContext context, EcUdc data)
{
  int ret = TRUE;
  
  switch (ecudc_type (udc))
  {
    case ENTC_UDC_LIST:
    {
      // must have transaction
    }
    break;
    case ENTC_UDC_NODE:
    {
      ret = adbo_node_update (udc, filter, context, data, TRUE);
    }
    break;
  }
  
  return ret;  
}

//----------------------------------------------------------------------------------------

int adbo_node_delete (EcUdc node, EcUdc filter, AdboContext context, int withTransaction);

int adbo_delete (EcUdc udc, EcUdc filter, AdboContext context)
{
  int ret = TRUE;
  
  switch (ecudc_type (udc))
  {
    case ENTC_UDC_LIST:
    {
      // must have transaction
    }
      break;
    case ENTC_UDC_NODE:
    {
      ret = adbo_node_delete (udc, filter, context, TRUE);
    }
      break;
  }
  
  return ret;  
}

//----------------------------------------------------------------------------------------

EcUdc adbo_node_values (EcUdc node);

EcUdc adbo_item_values (EcUdc item)
{
  return adbo_node_values (item);
}

//----------------------------------------------------------------------------------------

int adbo_clear (EcUdc node)
{
  return TRUE;
}

//----------------------------------------------------------------------------------------

