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
#include "adbo_node.h"
#include "adbo_context_intern.h"
#include "adbo_data.h"
#include "adbo_context.h"

// entc includes
#include <sys/entc_mutex.h>
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

int adbo_item_cursor (AdboContext context, EcCursor cursor, EcUdc item, EcUdc filter)
{
  return adbo_node_cursor (context, cursor, item, filter);
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
      ret = adbo_node_update (udc, filter, context, data, FALSE);
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

//========================================================================================

struct Adbo_s
{
  AdboContext adboctx;
  
  EcUdc root;
  
  EntcMutex mutex;
  
};

//----------------------------------------------------------------------------------------

Adbo adbo_create (const EcString confPath, const EcString binPath, const EcString objFile)
{
  EcXMLStream xmlstream;
  
  Adbo self = ENTC_NEW (struct Adbo_s);
  
  self->adboctx = adbo_context_createJson (confPath, binPath);
  self->root = NULL;
  self->mutex = entc_mutex_new ();
  
  xmlstream = ecxmlstream_openfile (objFile, confPath);
  if (xmlstream)
  {
    /* parse the xml structure */
    while( ecxmlstream_nextNode( xmlstream ) )
    {
      if( ecxmlstream_isBegin( xmlstream, "nodes" ) )
      {
        self->root = adbo_structure_fromXml (self->adboctx, xmlstream, "root", "nodes");
      }
    }
    
    ecxmlstream_destroy (&xmlstream);
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_destroy (Adbo* pself)
{
  Adbo self = *pself;
  
  if (self->root)
  {
    ecudc_destroy(EC_ALLOC, &(self->root));
  }
  
  adbo_context_destroy (&(self->adboctx));
  entc_mutex_del (&(self->mutex));
  
  ENTC_DEL (pself, struct Adbo_s);
}

//----------------------------------------------------------------------------------------

int adbo_db_update (Adbo self, const EcString table, EcUdc* data, EcUdc caseNode)
{
  int res = TRUE;
  EcUdc tableNode;
  EcUdc dataNode;
  EcUdc dataTable;
  EcUdc params;
  
  entc_mutex_lock (self->mutex);
  
  if (self->root == NULL)
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "insert", "[%s] root is NULL", table);

    return FALSE;
  }
  
  //eclog_fmt (LL_TRACE, "ADBO", "insert", "[%s] #1", table);

  tableNode = adbo_get_table (self->root, table);
  if (isNotAssigned (tableNode))
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "insert", "[%s] table node is NULL", table);

    return FALSE;
  }
  
  dataNode = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  dataTable = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, table);

  ecudc_add (dataTable, data);
  ecudc_add (dataNode, &dataTable);

  //eclog_fmt (LL_TRACE, "ADBO", "insert", "[%s] #2", table);

  if (caseNode == NULL)
  {
    params = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  }
  else
  {
    params = caseNode;
  }
  
  // debug
  {
    EcString h = ecjson_toString (dataNode);
    
    //eclog_fmt (LL_TRACE, "ADBO", "insert D", h);

    ecstr_delete(&h);
  }
  // debug
  {
    EcString h = ecjson_toString (params);
    
    //eclog_fmt (LL_TRACE, "ADBO", "insert P", h);
    
    ecstr_delete(&h);
  }
  
  res = adbo_update (tableNode, params, self->adboctx, dataNode);

  //eclog_fmt (LL_TRACE, "ADBO", "insert", "[%s] done -> %i", table, res);

  ecudc_destroy (EC_ALLOC, &dataNode);
  
  if (caseNode == NULL)
  {
    ecudc_destroy (EC_ALLOC, &params);
  }
  
  entc_mutex_unlock (self->mutex);
  
  //eclog_fmt (LL_TRACE, "ADBO", "insert", "[%s] #3", table);

  return res;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_db_cursor (Adbo self, const EcString table, EcUdc params)
{
  EcUdc tableNode;
  EcUdc cursorNode;

  entc_mutex_lock (self->mutex);
  
  if (self->root == NULL)
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "insert", "[%s] root is NULL", table);
    
    return NULL;
  }

  tableNode = adbo_get_table (self->root, table);
  if (isNotAssigned (tableNode))
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "insert", "[%s] table node is NULL", table);
    
    return NULL;
  }

  cursorNode = ecudc_create (EC_ALLOC, ENTC_UDC_CURSOR, NULL);
  
  {
    EcCursor cursor = ecudc_asCursor (cursorNode);
    
    adbo_item_cursor (self->adboctx, cursor, tableNode, params);
  }
  
  entc_mutex_unlock (self->mutex);

  return cursorNode;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_db_fetch (Adbo self, const EcString table, EcUdc params)
{
  EcUdc tableNode;
  EcUdc values;

  entc_mutex_lock (self->mutex);
  
  if (self->root == NULL)
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "insert", "[%s] root is NULL", table);
    
    return NULL;
  }

  tableNode = adbo_get_table (self->root, table);
  if (isNotAssigned (tableNode))
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "insert", "[%s] table node is NULL", table);
    
    return NULL;
  }
  
  adbo_item_fetch (tableNode, params, self->adboctx);
  
  values = adbo_item_values (tableNode);

  entc_mutex_unlock (self->mutex);

  return values;
}

//----------------------------------------------------------------------------------------

int adbo_db_delete (Adbo self, const EcString table, EcUdc params)
{
  EcUdc tableNode;

  entc_mutex_lock (self->mutex);

  if (self->root == NULL)
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "delete", "[%s] root is NULL", table);
    
    return ENTC_ERR_PROCESS_FAILED;
  }

  tableNode = adbo_get_table (self->root, table);
  if (isNotAssigned (tableNode))
  {
    entc_mutex_unlock (self->mutex);
    
    eclog_fmt (LL_ERROR, "ADBO", "delete", "[%s] table node is NULL", table);
    
    return ENTC_ERR_PROCESS_FAILED;
  }

  adbo_delete (tableNode, params, self->adboctx);

  entc_mutex_unlock (self->mutex);
  
  return ENTC_ERR_NONE;
}

//----------------------------------------------------------------------------------------

struct AdblManager_s* adbo_db_adbl (Adbo self)
{
  return self->adboctx->adblm;
}

//----------------------------------------------------------------------------------------
