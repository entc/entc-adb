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

//----------------------------------------------------------------------------------------

// forward declarations
void adbo_node_fromXml (EcUdc, AdboContext, EcXMLStream);
void adbo_item_fromXml (EcUdc, AdboContext, EcXMLStream, const EcString tag);

//----------------------------------------------------------------------------------------

void adbo_structure_items_fromXml (EcUdc items, AdboContext context, EcXMLStream xmlstream, const EcString tag)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "node"))
  {
    eclogger_log(context->logger, LL_TRACE, "ADBO", "{parse} found node");
    
    EcUdc udc = ecudc_create (ENTC_UDC_NODE, "node");
    adbo_node_fromXml (udc, context, xmlstream);
    
    ecudc_add (items, &udc);
  }
  else if (ecxmlstream_isBegin (xmlstream, "substitute"))
  {
    
  }
  else if (ecxmlstream_isBegin (xmlstream, "item"))
  {
    eclogger_log(context->logger, LL_TRACE, "ADBO", "{parse} found item");
    
    EcUdc udc = ecudc_create (ENTC_UDC_NODE, "item");
    adbo_item_fromXml (udc, context, xmlstream, "item");

    ecudc_add (items, &udc);
  }
  
  ENTC_XMLSTREAM_END (tag)  
}

//----------------------------------------------------------------------------------------

EcUdc adbo_structure_fromXml (AdboContext context, EcXMLStream xmlstream, const EcString name, const EcString tag)
{
  EcUdc items = ecudc_create(ENTC_UDC_LIST, name);
  
  adbo_structure_items_fromXml (items, context, xmlstream, tag);
  
  return items;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_structure_fromDatabase (AdboContext context)
{
  return NULL;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_get_table (EcUdc items, const EcString tablename)
{
  switch (ecudc_type (items))
  {
    case ENTC_UDC_LIST:
    {
      void* cursor = NULL;
      EcUdc item;      
      // iterrate through all nodes
      for (item = ecudc_next (items, &cursor); isAssigned (item); item = ecudc_next (items, &cursor))
      {
        const EcString dbtable = ecudc_get_asString(item, ".dbtable", NULL);
        if (ecstr_equal(dbtable, tablename))
        {
          return item;
        }
      }      
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

int adbo_node_fetch (EcUdc node, EcUdc data, AdboContext context);

int adbo_fetch (EcUdc udc, EcUdc data, AdboContext context)
{
  int ret = TRUE;
  
  switch (ecudc_type (udc))
  {
    case ENTC_UDC_LIST:
    {
      void* cursor = NULL;
      EcUdc item;
      // iterrate through all nodes
      for (item = ecudc_next (udc, &cursor); isAssigned (item); item = ecudc_next (udc, &cursor))
      {
        ret = ret && adbo_node_fetch (item, data, context);
      }  
    }
    break;
    case ENTC_UDC_NODE:
    {
      ret = adbo_node_fetch (udc, data, context);
    }
    break;
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_update (EcUdc udc, EcUdc filter, AdboContext context, EcUdc data)
{
  
}

//----------------------------------------------------------------------------------------

int adbo_delete (EcUdc udc, EcUdc filter, AdboContext context)
{
  
}

//----------------------------------------------------------------------------------------
