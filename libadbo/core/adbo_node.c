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

#include "adbo_object.h"
#include "adbo_schema.h"
#include "adbo_container.h"
#include "adbo_context_intern.h"

#include <types/eclist.h>
#include <adbl.h>
#include <adbl_structs.h>

#include <system/macros.h>
#include "adbo_types.h"

#include <tools/ecdata.h>
#include <tools/ecjson.h>

#include <system/ectime.h>

struct AdboNodePart_s {
  
  AdboContainer container;
  
  int state;
  
  EcList primary_keys;

};

typedef struct AdboNodePart_s* AdboNodePart;

struct AdboNode_s
{

  AdboNodePart spart;
  
  EcString dbsource;

  EcString dbtable;
  
  EcList foreign_keys;

  EcList parts;

  uint_t max;

  uint_t min;
  
}; typedef struct AdboNode_s* AdboNode;

//----------------------------------------------------------------------------------------

EcUdc adbo_value_fromXml (AdboContext context, EcXMLStream xmlstream);

void adbo_dbkeys_fromXml (EcUdc dbkeys, AdboContext context, EcXMLStream xmlstream, const EcString tag)
{  
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "value"))
  {
    EcUdc key = adbo_value_fromXml (context, xmlstream);
    
    ecudc_add (dbkeys, &key);
  }
  
  ENTC_XMLSTREAM_END (tag)  
}

//----------------------------------------------------------------------------------------

void adbo_item_fromXml (EcUdc cols, AdboContext context, EcXMLStream xmlstream, const EcString tag);

void adbo_node_structure_fromXml (EcUdc cols, AdboContext context, EcXMLStream xmlstream, const EcString tag)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "item"))
  {
    adbo_item_fromXml (cols, context, xmlstream, "item");
  }
  
  ENTC_XMLSTREAM_END (tag)  
}

//----------------------------------------------------------------------------------------

void adbo_node_fromXml (EcUdc rootNode, AdboContext context, EcXMLStream xmlstream)
{
  if (isAssigned (xmlstream))
  {
    // add a default item
    EcUdc node = ecnode_create_item (rootNode, ecxmlstream_nodeAttribute (xmlstream, "dbtable"));
    
    ENTC_XMLSTREAM_BEGIN
    
    if (ecxmlstream_isBegin (xmlstream, "objects"))
    {
      EcUdc cols = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, ECDATA_COLS);

      adbo_node_structure_fromXml (cols, context, xmlstream, "objects");

      ecudc_add (node, &cols);
    }
    else if (ecxmlstream_isBegin (xmlstream, "primary_keys"))
    {
      EcUdc ids = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, ECDATA_IDS);
      
      adbo_dbkeys_fromXml (ids, context, xmlstream, "primary_keys");
      
      ecudc_add (node, &ids);
    }
    else if (ecxmlstream_isBegin (xmlstream, "foreign_keys"))
    {
      EcUdc refs = ecudc_node (node, ECDATA_REFS);
      if (isAssigned (refs))
      {
        adbo_dbkeys_fromXml (refs, context, xmlstream, "foreign_keys");
      }
      else
      {
        refs = ecudc_create(EC_ALLOC, ENTC_UDC_NODE, ECDATA_REFS);
        
        adbo_dbkeys_fromXml (refs, context, xmlstream, "foreign_keys");
        
        ecudc_add (node, &refs);
      }      
    }
    else if (ecxmlstream_isBegin (xmlstream, "value"))
    {
    }
    else if (ecxmlstream_isBegin (xmlstream, "data"))
    {
      
    }    
    
    ENTC_XMLSTREAM_END ("node")
  }
}

//----------------------------------------------------------------------------------------

int adbo_dbkeys_set_constraint (EcUdc item, AdblConstraint* constraint, const EcString dbcolumn, int first);

int adbo_dbkeys_set_constraint_list (EcUdc item, AdblConstraint* constraint, const EcString dbcolumn)
{
  EcUdc var; void* cursor = NULL;
  for (var = ecudc_next (item, &cursor); isAssigned (var); var = ecudc_next (item, &cursor))
  {
    if (!adbo_dbkeys_set_constraint (var, constraint, dbcolumn, FALSE))
    {
      return FALSE;
    }
  }  
  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_dbkeys_set_constraint (EcUdc item, AdblConstraint* constraint, const EcString dbcolumn, int first)
{
  switch (ecudc_type (item))
  {
    case ENTC_UDC_LIST:
    {
      if (first)
      {
        AdblConstraint* h = adbl_constraint_new (QUOMADBL_CONSTRAINT_IN);

        if (!adbo_dbkeys_set_constraint_list (item, h, dbcolumn))
        {
          adbl_constraint_delete(&h);
          return FALSE;
        }
        else
        {
          adbl_constraint_addConstraint (constraint, h);
          h = NULL;          
        }
      }
      else
      {
        return adbo_dbkeys_set_constraint_list (item, constraint, dbcolumn);
      }
    }
    break;
    case ENTC_UDC_STRING:
    {
      const EcString h = ecudc_asString (item);        
      if (ecstr_empty(h))
      {
        //eclogger_fmt (LL_WARN, "ADBO", "dkkey", "key '%s' has empty value", dbcolumn); 
        return FALSE;          
      }
      
      //eclogger_fmt (LL_TRACE, "ADBO", "dbkey", "add key '%s' with value '%s'", dbcolumn, h);
      
      adbl_constraint_addChar(constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, h);            
    }
    break;
    case ENTC_UDC_BYTE:
    {
      adbl_constraint_addLong (constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asByte (item));
    }
    break;
    case ENTC_UDC_UINT32:
    {
      adbl_constraint_addLong (constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asUInt32 (item));
    }
    break;
    case ENTC_UDC_INT32:
    {
      int res;
      adbl_constraint_addLong (constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asInt32 (item, &res));
    }
    break;
    case ENTC_UDC_UINT64:
    {
      adbl_constraint_addLong (constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asUInt64 (item));
    }
    break;
    case ENTC_UDC_INT64:
    {
      adbl_constraint_addLong (constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, ecudc_asInt64 (item));
    }
    break;
    default:
    {
      eclogger_fmt (LL_WARN, "ADBO", "dkkey", "key '%s' has not supported type", dbcolumn); 
      return FALSE;        
    }
    break;
  }  
  
  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_dbkeys_value_contraint_add (EcUdc value, EcUdc data, AdblConstraint* constraint, AdblQuery* query)
{
  // variables
  const EcString dbcolumn = ecudc_name (value);
  if (isNotAssigned (dbcolumn))
  {
    eclogger_fmt (LL_WARN, "ADBO", "dkkey", "key has no dbcolumn definition"); 
    return FALSE;
  }

  if (isAssigned (query))
  {
    adbl_query_addColumn (query, dbcolumn, 0);
  }
  
  // check if we have a data value with the same name as the dbcolumn
  if (isNotAssigned (data))
  {
    //eclogger_fmt (LL_TRACE, "ADBO", "dkkey", "no constraints in data");
    return FALSE;
  }  
  
  {
    EcUdc dataConstraint = ecudc_node (data, dbcolumn); 
    
    if (isNotAssigned (dataConstraint))
    {
      //eclogger_fmt (LL_TRACE, "ADBO", "dkkey", "key '%s' no value found", dbcolumn);
      return FALSE;
    }
    else
    {
      return adbo_dbkeys_set_constraint (dataConstraint, constraint, dbcolumn, TRUE);
    }
  }
      
  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_dbkeys_constraints (EcUdc dbkeys, EcUdc data, AdblConstraint* constraint, AdblQuery* query)
{
  int ret = TRUE;
  
  void* cursor = NULL;
  EcUdc dbkey;

  // for debug
  /*
  {
    EcString jsontext = ecjson_write (dbkeys);
    eclogger_fmt (LL_TRACE, "ADBO", "dbkeys", ecstr_cstring(jsontext));
    ecstr_delete(&jsontext);
  }
   */
  
  for (dbkey  = ecudc_next (dbkeys, &cursor); isAssigned (dbkey); dbkey = ecudc_next (dbkeys, &cursor))
  {
    int h = adbo_dbkeys_value_contraint_add (dbkey, data, constraint, query);
    ret = ret && h;
  }

  return ret;
}
  
//----------------------------------------------------------------------------------------

AdblConstraint* adbo_node_constraints (EcUdc node, EcUdc data, AdboContext context, AdblQuery* query)
{
  AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  // check primary keys and foreign keys if exists and add them
    
  EcUdc prikeys = ecudc_node (node, ECDATA_IDS);
  if (isAssigned (prikeys))
  {
    //eclogger_fmt (LL_TRACE, "ADBO", "constraints", "add primary keys");
    adbo_dbkeys_constraints (prikeys, data, constraint, query);
  }  
  
  if (adbl_constraint_empty (constraint))
  {
    EcUdc fkeys, cols;
    
    fkeys = ecudc_node (node, ECDATA_REFS);
    if (isAssigned (fkeys))
    {
      adbo_dbkeys_constraints (fkeys, data, constraint, query);
    }
    
    cols = ecudc_node (node, ECDATA_COLS);
    if (isAssigned (cols))
    {
      adbo_dbkeys_constraints (cols, data, constraint, NULL);
    }    
  }
  // clean up
  if (adbl_constraint_empty (constraint))
  {
    adbl_constraint_delete (&constraint);
  }
  
  return constraint;    
}

//----------------------------------------------------------------------------------------

void adbo_node_primary_sequence (EcUdc node, AdblSession dbsession, const EcString dbtable, EcUdc filter,EcUdc values, AdblAttributes* attrs)
{
  // check only primary key
  EcUdc prikeys = ecudc_node (node, ECDATA_IDS);
  if (isAssigned (prikeys))
  {
    void* cursor = NULL;
    EcUdc dbkey;
    
    for (dbkey  = ecudc_next (prikeys, &cursor); isAssigned (dbkey); dbkey = ecudc_next (prikeys, &cursor))
    {
      const EcString dbcolumn = ecudc_name (dbkey);
      if (isAssigned (dbcolumn))
      {
        AdblSequence* seq = adbl_dbsequence_get(dbsession, dbtable);
        
        uint32_t id = adbl_sequence_next (seq);
        
        adbl_attrs_addLong(attrs, dbcolumn, id);
        
        ecudc_add_asUInt32(EC_ALLOC, values, dbcolumn, id);        
        ecudc_add_asUInt32(EC_ALLOC, filter, dbcolumn, id);
      }
    }
  }
}

//----------------------------------------------------------------------------------------

int adbo_node_primary (EcUdc node, EcUdc data, AdboContext context, AdblConstraint* constraint)
{
  int ret = FALSE;
  
  // check only primary key
  EcUdc prikeys = ecudc_node (node, ECDATA_IDS);
  if (isAssigned (prikeys))
  {
    ret = adbo_dbkeys_constraints (prikeys, data, constraint, NULL);
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_foreign (EcUdc node, EcUdc data, AdboContext context, AdblConstraint* constraint)
{
  // check only primary key
  EcUdc fkeys = ecudc_node (node, ECDATA_REFS);
  if (isAssigned (fkeys))
  {
    return adbo_dbkeys_constraints (fkeys, data, constraint, NULL);
  }  
  return FALSE;  
}

//----------------------------------------------------------------------------------------

EcUdc adbo_node_parts (EcUdc node)
{
  return ecudc_node(node, "parts");
}

//----------------------------------------------------------------------------------------

void adbo_node_dbquery_columns (EcUdc node, AdblQuery* query)
{
  void* cursor = NULL;
  EcUdc value;
  // ignore parts just collect all info from items
  EcUdc cols = ecudc_node (node, ECDATA_COLS);
  if (isNotAssigned (cols))
  {
    return; 
  }
  // iterrate through all items to get info
  for (value = ecudc_next (cols, &cursor); isAssigned (value); value = ecudc_next (cols, &cursor))
  {
    // variables
    const EcString dbcolumn = ecudc_name (value);
    if (isNotAssigned (dbcolumn))
    {
      continue;
    }
    
    adbl_query_addColumn (query, dbcolumn, 0);
  }  
}

//----------------------------------------------------------------------------------------

int adbo_node_dbquery_cursor (EcUdc node, EcUdc values, ulong_t dbmin, AdboContext context, AdblCursor* cursor, AdblQuery* query)
{
  int ret = TRUE;
  
  // multi mode
  while (adbl_dbcursor_next (cursor)) 
  {
    int colno = 0;
    EcListNode c;

    EcUdc items = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);

    for (c = eclist_first(query->columns); c != eclist_end(query->columns); c = eclist_next(c), colno++)
    {
      AdblQueryColumn* qc = eclist_data(c);      
      ecudc_add_asString (EC_ALLOC, items, qc->column, adbl_dbcursor_data(cursor, colno));          
    }
    
    ecudc_add (values, &items);
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

#define PREFETCHED_CACHE 20

typedef struct 
{
  
  AdblSession session;
  
  AdblCursor* dbcursor;
  
  AdblQuery* dbquery;
  
  int columns;
  
  int done;
  
  EcTable data;
  
} AdboCursorData; 

//----------------------------------------------------------------------------------------

int _STDCALL adbo_cursor_fill (void* ptr, EcTable* table)
{
  int row = 0;
  int col;
  AdboCursorData* self = ptr;

  *table = NULL;

  if (self->done)
  {
    return 0; // this means no data anymore
  }
    
  while (adbl_dbcursor_next (self->dbcursor))
  {
    row++;  // starts by row 1 to ignore header

    for (col = 0; col < self->columns; col++)
    {
      EcString current = ectable_get (self->data, row, col);
      
      ecstr_replace (&current, adbl_dbcursor_data (self->dbcursor, col));
      
      ectable_set (self->data, row, col, current);

      //eclogger_fmt (LL_TRACE, "ADBO", "cursor", "set value [%i][%i] '%s'", row, col, current);
    }
    
    if (row >= PREFETCHED_CACHE)
    {
      *table = self->data;

      //eclogger_fmt (LL_TRACE, "ADBO", "cursor", "fetched %i rows", row);
      return row;
    }
  }

  if (row > 0)
  {
    *table = self->data;
  }
  
  self->done = TRUE;
  
  //eclogger_fmt (LL_TRACE, "ADBO", "cursor", "fetched %i rows", row);
  return row;
}

//----------------------------------------------------------------------------------------

AdboCursorData* adbo_cursordata_create (AdblSession* dbsession, AdblQuery** dbquery, AdblCursor** dbcursor)
{  
  AdboCursorData* self = ENTC_NEW (AdboCursorData);
  
  self->session = *dbsession;
  *dbsession = NULL;
  
  self->dbquery = *dbquery;
  *dbquery = NULL;

  self->dbcursor = *dbcursor;
  *dbcursor = NULL;
  
  // create table
  self->columns = eclist_size (self->dbquery->columns);
  
  self->data = ectable_new (self->columns, PREFETCHED_CACHE + 1);
  self->done = FALSE;
  
  // fill header of the table
  {
    int n = 0;
    EcListNode c;
    
    for (c = eclist_first (self->dbquery->columns); c != eclist_end (self->dbquery->columns); c = eclist_next(c), n++)
    {
      AdblQueryColumn* qc = eclist_data(c);      
      ectable_set (self->data, 0, n, ecstr_copy(qc->column));
    }    
  }
  
  //eclogger_fmt (LL_TRACE, "ADBO", "cursor", "created with %i columns", self->columns);
  
  return self;
}

//----------------------------------------------------------------------------------------

int _STDCALL adbo_cursor_destroy (void* ptr, EcTable table)
{
  AdboCursorData* self = ptr;
  
  //eclogger_msg (LL_TRACE, "ADBO", "cursor", "destroy and clean cursor");
  
  adbl_query_delete (&(self->dbquery));
  
  adbl_dbcursor_release (&(self->dbcursor));

  adbl_closeSession (&(self->session));
  
  // clean table
  {
    EcTableNode node;
    
    for (node = ectable_first(self->data); node != ectable_end(self->data); node = ectable_next(self->data, node))
    {
      int c;
      
      for (c = 0; c < self->columns; c++)
      {
        EcString current = ectable_data (self->data, node, c);
        ecstr_delete (&current);
      }
    }
    
    ectable_delete (&(self->data));
  }
  
  ENTC_DEL (&self, AdboCursorData);
  
  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_node_dbquery2 (AdboContext context, EcCursor cursor, ulong_t dbmin, AdblSession* session, AdblQuery** query)
{
  AdblSecurity adblsec;
  AdblCursor* dbcursor;

  dbcursor = adbl_dbquery (*session, *query, &adblsec);
  if (isAssigned (dbcursor))
  {
    AdboCursorData* cdata = adbo_cursordata_create (session, query, &dbcursor);
    
    eccursor_callbacks(cursor, cdata, adbo_cursor_fill, adbo_cursor_destroy);
    
    return TRUE;
  }
  
  return FALSE;
}

//----------------------------------------------------------------------------------------

int adbo_node_dbquery (EcUdc node, EcUdc parts, ulong_t dbmin, AdboContext context, AdblSession session, AdblQuery* query)
{
  AdblSecurity adblsec;
  AdblCursor* cursor;
  int ret = TRUE;
  
  adbo_node_dbquery_columns (node, query);
  // execute sql query
  //eclogger_msg (LL_TRACE, "ADBO", "query", "execute query");

  cursor = adbl_dbquery (session, query, &adblsec);
  if (isAssigned (cursor))
  {
    ret = adbo_node_dbquery_cursor (node, parts, dbmin, context, cursor, query);
    
    adbl_dbcursor_release (&cursor);
  }  
  
  return ret;  
}

//----------------------------------------------------------------------------------------

EcUdc adbo_node_values (EcUdc node)
{
  return ecudc_node (node, ECDATA_ROWS);
}

//----------------------------------------------------------------------------------------

int adbo_node_cursor (AdboContext context, EcCursor cursor, EcUdc node, EcUdc data)
{
  int ret;
  // strings
  const EcString dbsource;
  const EcString dbtable;
  uint32_t dbmin;
  
  // adbl variables
  AdblSession dbsession;
  AdblQuery* query;
  AdblConstraint* constraints;

  if (isNotAssigned (cursor))
  {
    eclogger_msg (LL_ERROR, "ADBO", "cursor", "cursor is NULL");
    return FALSE;
  }
  
  if (isNotAssigned (node))
  {
    eclogger_msg (LL_ERROR, "ADBO", "cursor", "node is NULL");
    return FALSE;
  }
  
  dbtable = ecudc_name (node);
  if (isNotAssigned (dbtable))
  {
    eclogger_msg (LL_WARN, "ADBO", "cursor", "table name not defined");
    return FALSE;
  }

  dbmin = ecudc_get_asUInt32(node, ".dbmin", 1);

  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_fmt (LL_ERROR, "ADBO", "cursor", "can't connect to database '%s'", dbsource);
    return FALSE;
  }
  
  //eclogger_msg (LL_TRACE, "ADBO", "request", "prepare query");

  query = adbl_query_new ();

  // add some default stuff
  adbl_query_setTable (query, dbtable);
  
  // ***** check constraints *****
  // we can use spart here, because constrainst are the same for all parts
  constraints = adbo_node_constraints (node, data, context, query);

  if (isAssigned (constraints))
  {
    adbl_query_setConstraint (query, constraints);
  }
    
  adbo_node_dbquery_columns (node, query);
  
  ret = adbo_node_dbquery2 (context, cursor, dbmin, &dbsession, &query);
  
  if (isAssigned (constraints))
  {
    adbl_constraint_delete (&constraints);
  }

  //eclogger_fmt (LL_TRACE, "ADBO", "request", "query done [%i]", ret);
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_fetch (EcUdc node, EcUdc data, AdboContext context)
{
  // variables
  const EcString dbsource;
  AdblSession dbsession;
  EcUdc values;
  uint32_t dbmin;
  AdblConstraint* constraints;
  AdblQuery* query;

  int ret = TRUE;
  
  const EcString dbtable = ecudc_name (node);
  if (isNotAssigned (dbtable))
  {
    eclogger_msg (LL_WARN, "ADBO", "fetch", "table name not defined");
    return FALSE;
  }
  
  ecudc_del (EC_ALLOC, node, ECDATA_ROWS);
  
  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_fmt (LL_ERROR, "ADBO", "fetch", "can't connect to database '%s'", dbsource);
    return FALSE;
  }
  
  values = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, ECDATA_ROWS);
  
  dbmin = ecudc_get_asUInt32(node, ".dbmin", 1);
  
  
  query = adbl_query_new ();
  // add some default stuff
  //eclogger_msg (LL_TRACE, "ADBO", "request", "prepare query");
  
  // ***** check constraints *****
  // we can use spart here, because constrainst are the same for all parts
  constraints = adbo_node_constraints (node, data, context, query);
  if (isAssigned (constraints))
  {
    adbl_query_setConstraint (query, constraints);
    
    adbl_query_setTable (query, dbtable);
    // execute in extra method to avoid memory leaks in query and dbsession
    ret = adbo_node_dbquery (node, values, dbmin, context, dbsession, query);
    
    adbl_constraint_delete (&constraints);
  }
  else
  {
    adbl_query_setTable (query, dbtable);
    // execute in extra method to avoid memory leaks in query and dbsession
    ret = adbo_node_dbquery (node, values, dbmin, context, dbsession, query);    
  }
  
  adbl_query_delete (&query);
  
  adbl_closeSession (&dbsession);
  
  //eclogger_fmt (LL_TRACE, "ADBO", "request", "query done [%i]", ret);

  ecudc_add (node, &values);
  
  return ret;  
}

//----------------------------------------------------------------------------------------

void adbo_node_appendColumn (AdblAttributes* attrs, EcUdc columnItem, EcUdc dataItems, EcUdc values)
{
  const EcString columnName;
  EcUdc dataItem;  

  columnName = ecudc_name (columnItem);
  if (isNotAssigned (columnName))
  {
    eclogger_msg (LL_WARN, "ADBO", "attr", "value in items without dbcolumn");
    return;
  }

  dataItem = ecudc_node(dataItems, columnName);
  if (isNotAssigned (dataItem))
  {
    eclogger_fmt (LL_TRACE, "ADBO", "attr", "node has not item '%s'", columnName);
    return;
  }
  
  switch (ecudc_type (dataItem))
  {
    case ENTC_UDC_NODE:
    case ENTC_UDC_LIST:
    {
      EcString val = ecjson_write (dataItem);
      
      adbl_attrs_addChar (attrs, columnName, val);
      
      if (isAssigned(values))
      {
        ecudc_add_asS_o(EC_ALLOC, values, columnName, &val);
      }
      else
      {
        ecstr_delete (&val);
      }
    }
    break;
    case ENTC_UDC_STRING:
    {
      adbl_attrs_addChar (attrs, columnName, ecudc_asString (dataItem));
      
      if (isAssigned(values))
      {
        ecudc_add_asString (EC_ALLOC, values, columnName, ecudc_asString (dataItem));
      }
    }
    break;
    case ENTC_UDC_TIME:
    {
      eclogger_fmt (LL_TRACE, "ADBO", "attr", "add time type");

      const time_t* t = ecudc_asTime (dataItem);
      
      char buffer [100];
      ectime_toString (t, buffer, 100);
      
      adbl_attrs_addChar (attrs, columnName, buffer);
      
      if (isAssigned(values))
      {
        ecudc_add_asString (EC_ALLOC, values, columnName, buffer);
      }

      eclogger_fmt (LL_TRACE, "ADBO", "attr", "add time type");
    }
    break;
    case ENTC_UDC_INT64:
    {
      adbl_attrs_addLong (attrs, columnName, ecudc_asInt64 (dataItem));

      if (isAssigned(values))
      {
        ecudc_add_asInt64 (EC_ALLOC, values, columnName, ecudc_asInt64 (dataItem));
      }
    }
    break;
    case ENTC_UDC_UINT64:
    {
      adbl_attrs_addLong (attrs, columnName, ecudc_asUInt64 (dataItem));
      
      if (isAssigned(values))
      {
        ecudc_add_asUInt64 (EC_ALLOC, values, columnName, ecudc_asUInt64 (dataItem));
      }
    }
    break;
    case ENTC_UDC_INT32:
    {
      int res;
      adbl_attrs_addLong (attrs, columnName, ecudc_asInt32 (dataItem, &res));
      
      if (isAssigned(values))
      {
        int res;
        ecudc_add_asInt32 (EC_ALLOC, values, columnName, ecudc_asInt32 (dataItem, &res));
      }
    }
    break;
    case ENTC_UDC_UINT32:
    {
      adbl_attrs_addLong (attrs, columnName, ecudc_asUInt32 (dataItem));
      
      if (isAssigned(values))
      {
        ecudc_add_asUInt32 (EC_ALLOC, values, columnName, ecudc_asUInt32 (dataItem));
      }
    }
    break;
    case ENTC_UDC_INT16:
    {
      adbl_attrs_addLong (attrs, columnName, ecudc_asInt16 (dataItem));
      
      if (isAssigned(values))
      {
        ecudc_add_asInt16 (EC_ALLOC, values, columnName, ecudc_asInt16 (dataItem));
      }
    }
    break;
    case ENTC_UDC_UINT16:
    {
      adbl_attrs_addLong (attrs, columnName, ecudc_asUInt16 (dataItem));
      
      if (isAssigned(values))
      {
        ecudc_add_asUInt16 (EC_ALLOC, values, columnName, ecudc_asUInt16 (dataItem));
      }
    }
    break;
    default:
    {
      const EcString val = ecudc_asString (dataItem);
      if (val)
      {
        adbl_attrs_addChar (attrs, columnName, val);

        if (isAssigned(values))
        {
          ecudc_add_asString (EC_ALLOC, values, columnName, val);
        }
      }
      else
      {
        eclogger_fmt (LL_WARN, "ADBO", "attr", "column item '%s' has no value", columnName);
        return;
      }
    }
  }
}

//----------------------------------------------------------------------------------------

void adbo_node_insert_values (AdboContext context, EcUdc cols, EcUdc update_item, EcUdc values, AdblAttributes* attrs)
{
  EcUdc column;
  void* cursor = NULL;
  EcString update_value = ecstr_init ();
  
  for (column = ecudc_next (cols, &cursor); isAssigned (column); column = ecudc_next (cols, &cursor))
  {
    // variables
    adbo_node_appendColumn (attrs, column, update_item, values);
  }
  
  ecstr_delete (&update_value);
}

//----------------------------------------------------------------------------------------

int adbo_node_insert (EcUdc node, AdboContext context, AdblSession dbsession, const EcString dbtable, EcUdc filter, EcUdc update_data, EcUdc cols, AdblConstraint* constraint)
{
  void* cursor = NULL;

  EcUdc item_update;
  EcUdc values_list;

  int ret = TRUE;

  ecudc_del (EC_ALLOC, node, ECDATA_ROWS);
  values_list = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, ECDATA_ROWS);
  
  // iterate through all new values
  for (item_update = ecudc_next (update_data, &cursor); isAssigned (item_update); item_update = ecudc_next (update_data, &cursor))
  {
    // has the item a primary key definition
    if (adbo_node_primary (node, item_update, context, NULL))
    {
      // good now check if it is in the original
      eclogger_msg (LL_WARN, "ADBO", "update", "multi update not implemented yet");
      ret = FALSE;
    }
    else
    {
      EcUdc values = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      AdblSecurity adblsec;
      AdblAttributes* attrs = adbl_attrs_new ();
      // ok so we do need to insert a new one
      AdblInsert* insert = adbl_insert_new ();
            
      adbl_insert_setTable (insert, dbtable);
      // convert contraint to attribute
      if (isAssigned (constraint))
      {
        adbl_constraint_attrs (constraint, attrs);
      }
      adbl_insert_setAttributes (insert, attrs);

      adbo_node_primary_sequence (node, dbsession, dbtable, filter, values, attrs);
      // all additional values
      if (isAssigned (cols) && isAssigned (values))
      {
        adbo_node_insert_values (context, cols, item_update, values, attrs);
      }
      
      ret = adbl_dbinsert (dbsession, insert, &adblsec);
      
      adbl_insert_delete (&insert);
      
      adbl_attrs_delete (&attrs);
      
      ecudc_add (values_list, &values);
    }
  }

  ecudc_add (node, &values_list);
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update_single (AdboContext context, AdblSession dbsession, const EcString dbtable, EcUdc update_data, EcUdc cols, EcUdc values, AdblConstraint* constraint)
{
  int ret = TRUE;
  EcUdc column;
  EcUdc item_update;
  void* cursor = NULL;
  void* cursor_value = NULL;
  void* cursor_update = NULL;

  AdblAttributes* attrs;
  int isInsert = FALSE;

  EcUdc item_value;
  
  /*
  {
    EcString jsontext = ecjson_write (cols);    
    eclogger_fmt (LL_DEBUG, "ADBO", "update", "cols: %s", ecstr_cstring(jsontext));
    ecstr_delete(&jsontext);
  }
  
  {
    EcString jsontext = ecjson_write (values);    
    eclogger_fmt (LL_DEBUG, "ADBO", "update", "values: %s", ecstr_cstring(jsontext));
    ecstr_delete(&jsontext);
  }
  
  {
    EcString jsontext = ecjson_write (update_data);    
    eclogger_fmt (LL_DEBUG, "ADBO", "update", "updates: %s", ecstr_cstring(jsontext));
    ecstr_delete(&jsontext);
  }
   */
  
  item_update = ecudc_next (update_data, &cursor_update);  
  if (isNotAssigned (item_update))
  {
    eclogger_msg (LL_ERROR, "ADBO", "update", "update array is empty");
    return FALSE;
  }  
  
  attrs = adbl_attrs_new ();

  item_value = ecudc_next (values, &cursor_value);
  //if (isNotAssigned (item_value))
  {
    isInsert = TRUE;
    
    for (column = ecudc_next (cols, &cursor); isAssigned (column); column = ecudc_next (cols, &cursor))
    {
      adbo_node_appendColumn (attrs, column, item_update, NULL);
    }
  }
  /*
  else
  {
    for (column = ecudc_next (cols, &cursor); isAssigned (column); column = ecudc_next (cols, &cursor))
    {
      const EcString dbcolumn = ecudc_name (column);
      if (isNotAssigned (dbcolumn))
      {
        eclogger_msg (LL_WARN, "ADBO", "update", "column with empty name");
        continue;
      }
      
      // now we retrieve the item from original fetched and want to update
      const EcString update_value = ecudc_get_asString(item_update, dbcolumn, NULL);
      if (isNotAssigned (update_value))
      {
        eclogger_fmt (LL_TRACE, "ADBO", "update", "item '%s' has no value and will not be updated", dbcolumn);    
        continue;
      }
      
      //const EcString fetched_value = ecudc_get_asString(item_value, dbcolumn, NULL);
      
      //if (ecstr_equal(fetched_value, update_value))
      {
        // ignore if they are anyway the same
        //continue;
      }
      
      adbl_attrs_addChar (attrs, dbcolumn, update_value);            
    }      
  }
   */
  
  // do we have something to update?
  if (!adbl_attrs_empty(attrs))
  {
    AdblSecurity adblsec;
    AdblUpdate* update = adbl_update_new ();
    // create the statement
    adbl_update_setTable(update, dbtable);
    adbl_update_setConstraint(update, constraint);
    adbl_update_setAttributes(update, attrs);
    
    ret = adbl_dbupdate (dbsession, update, isInsert, &adblsec);
    
    adbl_update_delete(&update);
  }
  else
  {
    eclogger_msg (LL_WARN, "ADBO", "update", "nothing to update");    
  }
  
  adbl_attrs_delete(&attrs);
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update_state (EcUdc node, EcUdc filter, AdboContext context, AdblSession dbsession, const EcString dbtable, EcUdc update_data, EcUdc cols, EcUdc values)
{
  int ret = TRUE;
  
  AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);

  if (adbo_node_primary (node, filter, context, constraint))
  {
    //eclogger_msg (LL_TRACE, "ADBO", "update", "decided to update");

    if (adbo_node_foreign (node, filter, context, constraint))
    {
      
    }
    
    
    // explicitely check the values
    if (isAssigned (values) && isAssigned (cols))
    {
      // there is a primary key given: good it makes things easier
      ret = adbo_node_update_single (context, dbsession, dbtable, update_data, cols, values, constraint);            
    }
    else
    {
      eclogger_msg (LL_ERROR, "ADBO", "update", "values and items must be assigned for update");
      ret = FALSE;
    }
  }
  else if (adbo_node_foreign (node, filter, context, constraint))
  {
    // no primary key but foreign key, lets try to insert
    ret = adbo_node_insert (node, context, dbsession, dbtable, filter, update_data, cols, constraint);
  }
  else
  {
    // no primary key nor foreign key, lets try to insert
    ret = adbo_node_insert (node, context, dbsession, dbtable, filter, update_data, cols, NULL);
  }
  
  adbl_constraint_delete(&constraint);

  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update (EcUdc node, EcUdc filter, AdboContext context, EcUdc data, int withTransaction)
{
  // variables
  const EcString dbsource;
  AdblSession dbsession;
  EcUdc update_data;
  EcUdc cols;
  EcUdc values;

  int ret = FALSE;
  
  const EcString dbtable = ecudc_name (node);
  if (isNotAssigned (dbtable))
  {
    eclogger_msg (LL_ERROR, "ADBO", "fetch", ".dbtable not defined");
    return FALSE;
  }
  
  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_fmt (LL_ERROR, "ADBO", "update", "can't connect to database '%s'", dbsource);
    return FALSE;
  }
  
  if (isNotAssigned (data))
  {
    eclogger_msg (LL_WARN, "ADBO", "update", "no data given");
    return FALSE;
  }
  
  // check if the data fits the structure
  update_data = ecudc_node (data, dbtable);
  if (isNotAssigned (update_data))
  {
    eclogger_fmt (LL_WARN, "ADBO", "update", "missing data for table '%s'", dbtable);
    return FALSE;    
  }
  
  if (ecudc_type(update_data) != ENTC_UDC_LIST)
  {
    eclogger_fmt (LL_WARN, "ADBO", "update", "data for '%s' is not a list object", dbtable);
    return FALSE;    
  }
  
  cols = ecudc_node(node, ECDATA_COLS);

  // values can be empty
  values = ecudc_node (node, ECDATA_ROWS); 
  if (isNotAssigned (values))
  {
    if (!adbo_node_fetch (node, data, context))
    {
      eclogger_fmt (LL_WARN, "ADBO", "update", "try to fill empty values but failed for '%s'", dbtable);
      return FALSE; 
    }
    // try again
    values = ecudc_node (node, ECDATA_ROWS); 
  } 
  
  if (withTransaction)
  {
    adbl_dbbegin (dbsession);
  }
  
  ret = adbo_node_update_state (node, filter, context, dbsession, dbtable, update_data, cols, values);
    
  if (withTransaction)
  {
    // commit / rollback all changes
    if (ret)
    {
      adbl_dbcommit (dbsession);
    }
    else
    {
      adbl_dbrollback (dbsession);        
    }
  }
  
  adbl_closeSession (&dbsession);
  return ret;  
}
           
//----------------------------------------------------------------------------------------

int adbo_node_delete (EcUdc node, EcUdc filter, AdboContext context, int withTransaction)
{
  // variables
  const EcString dbsource;
  AdblSession dbsession;

  int ret = FALSE;

  const EcString dbtable = ecudc_name (node);  
  if (isNotAssigned (dbtable))
  {
    eclogger_msg (LL_ERROR, "ADBO", "fetch", ".dbtable not defined");
    return FALSE;
  }
  
  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_fmt (LL_ERROR, "ADBO", "update", "can't connect to database '%s'", dbsource);
    return FALSE;
  }
      
  if (withTransaction)
  {
    adbl_dbbegin (dbsession);
  }
  
  {
    AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
    
    //eclogger_fmt (LL_TRACE, "ADBO", "delete", "prepare constraints");

    if (adbo_node_primary (node, filter, context, constraint))
    {
      AdblSecurity adblsec;      
      AdblDelete* delete = adbl_delete_new ();

      adbl_delete_setConstraint (delete, constraint);
      
      adbl_delete_setTable (delete, dbtable);
      // execute in extra method to avoid memory leaks in query and dbsession
      ret = adbl_dbdelete (dbsession, delete, &adblsec);
      
      adbl_delete_delete(&delete);
    }
    else if (adbo_node_foreign (node, filter, context, constraint))
    {
      AdblSecurity adblsec;      
      AdblDelete* delete = adbl_delete_new ();
      
      adbl_delete_setConstraint (delete, constraint);
      
      adbl_delete_setTable (delete, dbtable);
      // execute in extra method to avoid memory leaks in query and dbsession
      ret = adbl_dbdelete (dbsession, delete, &adblsec);
      
      adbl_delete_delete(&delete);      
    }
    else
    {
      eclogger_msg (LL_ERROR, "ADBO", "delete", "needs valid primary or foreign keys");
      ret = FALSE;
    }

    adbl_constraint_delete(&constraint);
  }
  
  if (withTransaction)
  {
    // commit / rollback all changes
    if (ret)
    {
      adbl_dbcommit (dbsession);
    }
    else
    {
      adbl_dbrollback (dbsession);        
    }
  }
  
  adbl_closeSession (&dbsession);
  return ret;
}

//----------------------------------------------------------------------------------------

void adbo_node_updateSize (EcUdc node, AdboContext context)
{
  // variables
  const EcString dbsource;
  AdblSession dbsession;

  EcUdc size = ecudc_node (node, ECDATA_SIZE);

  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_fmt (LL_ERROR, "ADBO", "update", "can't connect to database '%s'", dbsource);
    return;
  }

  if (isAssigned (size))
  {
    ecudc_setUInt64 (size, adbl_table_size (dbsession, ecudc_name (node)));
  }
  else
  {
    ecudc_add_asUInt64 (EC_ALLOC, node, ECDATA_SIZE, adbl_table_size (dbsession, ecudc_name (node)));
  }
}

//----------------------------------------------------------------------------------------
