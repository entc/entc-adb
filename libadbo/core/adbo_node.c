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

EcUdc adbo_dbkeys_fromXml (AdboContext context, EcXMLStream xmlstream, const EcString name, const EcString tag)
{
  EcUdc keys = ecudc_create(ENTC_UDC_LIST, name);
  
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "value"))
  {
    EcUdc key = adbo_value_fromXml (context, xmlstream);
    
    ecudc_add (keys, &key);
  }
  
  ENTC_XMLSTREAM_END (tag)  
  
  return keys;
}

//----------------------------------------------------------------------------------------

EcUdc adbo_structure_fromXml (AdboContext context, EcXMLStream xmlstream, const EcString name, const EcString tag);

void adbo_node_fromXml (EcUdc node, AdboContext context, EcXMLStream xmlstream)
{
  if (isAssigned (xmlstream))
  {
    ecudc_add_asString(node, ".dbtable", ecxmlstream_nodeAttribute (xmlstream, "dbtable"));

    ENTC_XMLSTREAM_BEGIN
    
    if (ecxmlstream_isBegin (xmlstream, "objects"))
    {
      EcUdc object = adbo_structure_fromXml (context, xmlstream, "items", "objects");
      
      ecudc_add (node, &object);
    }
    else if (ecxmlstream_isBegin (xmlstream, "primary_keys"))
    {
      EcUdc fkeys = adbo_dbkeys_fromXml (context, xmlstream, ".primary", "primary_keys");
      ecudc_add (node, &fkeys);
    }
    else if (ecxmlstream_isBegin (xmlstream, "foreign_keys"))
    {
      EcUdc fkeys = adbo_dbkeys_fromXml (context, xmlstream, ".fkeys", "foreign_keys");
      ecudc_add (node, &fkeys);
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

int adbo_dbkeys_value_contraint_add (EcUdc value, EcUdc data, AdblConstraint* constraint, EcLogger logger, AdblQuery* query)
{
  // variables
  const EcString data_value;

  const EcString dbcolumn = ecudc_get_asString(value, ".dbcolumn", NULL);
  if (isNotAssigned (dbcolumn))
  {
    eclogger_logformat (logger, LL_WARN, "ADBO", "{dkkey} key has no dbcolumn definition"); 
    return FALSE;
  }

  if (isAssigned (query))
  {
    adbl_query_addColumn(query, dbcolumn, 0);
  }
  // check if we have a data value with the same name as the dbcolumn
  data_value = ecudc_get_asString(data, dbcolumn, NULL);
  if (isNotAssigned (data_value))
  {
    eclogger_logformat (logger, LL_WARN, "ADBO", "{dkkey} key '%s' no value found", dbcolumn); 
    return FALSE;
  }
    
  if (isAssigned(constraint))
  {
    adbl_constraint_addChar(constraint, dbcolumn, QUOMADBL_CONSTRAINT_EQUAL, data_value);    
  }
  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_dbkeys_constraints (EcUdc dbkeys, EcUdc data, AdblConstraint* constraint, EcLogger logger, AdblQuery* query)
{
  int ret = TRUE;
  
  void* cursor = NULL;
  EcUdc dbkey;
  
  for (dbkey  = ecudc_next (dbkeys, &cursor); isAssigned (dbkey); dbkey = ecudc_next (dbkeys, &cursor))
  {
    ret = ret && adbo_dbkeys_value_contraint_add (dbkey, data, constraint, logger, query);
  }
  
  return ret;
}
  
//----------------------------------------------------------------------------------------

AdblConstraint* adbo_node_constraints (EcUdc node, EcUdc data, AdboContext context, AdblQuery* query)
{
  AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  // check primary keys and foreign keys if exists and add them
  EcUdc prikeys = ecudc_node (node, ".primary");
  if (isAssigned (prikeys))
  {
    adbo_dbkeys_constraints (prikeys, data, constraint, context->logger, query);
  }  
  if (adbl_constraint_empty (constraint))
  {
    EcUdc fkeys = ecudc_node (node, ".fkeys");
    if (isAssigned (fkeys))
    {
      adbo_dbkeys_constraints (fkeys, data, constraint, context->logger, query);
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

void adbo_node_primary_sequence (EcUdc node, AdblSession dbsession, const EcString dbtable, EcUdc values, AdblAttributes* attrs)
{
  // check only primary key
  EcUdc prikeys = ecudc_node (node, ".primary");
  if (isAssigned (prikeys))
  {
    void* cursor = NULL;
    EcUdc dbkey;
    
    for (dbkey  = ecudc_next (prikeys, &cursor); isAssigned (dbkey); dbkey = ecudc_next (prikeys, &cursor))
    {
      const EcString dbcolumn = ecudc_get_asString(dbkey, ".dbcolumn", NULL);
      if (isAssigned (dbcolumn))
      {
        AdblSequence* seq = adbl_dbsequence_get(dbsession, dbtable);
        
        uint_t id = adbl_sequence_next (seq);
        
        adbl_attrs_addLong(attrs, dbcolumn, id);
        
        ecudc_add_asL(values, dbcolumn, id);        
      }
    }
  }
}

//----------------------------------------------------------------------------------------

int adbo_node_primary (EcUdc node, EcUdc data, AdboContext context, AdblConstraint* constraint)
{
  // check only primary key
  EcUdc prikeys = ecudc_node (node, ".primary");
  if (isAssigned (prikeys))
  {
    return adbo_dbkeys_constraints (prikeys, data, constraint, context->logger, NULL);
  }  
  return FALSE;
}

//----------------------------------------------------------------------------------------

int adbo_node_foreign (EcUdc node, EcUdc data, AdboContext context, AdblConstraint* constraint)
{
  // check only primary key
  EcUdc prikeys = ecudc_node (node, ".fkeys");
  if (isAssigned (prikeys))
  {
    return adbo_dbkeys_constraints (prikeys, data, constraint, context->logger, NULL);
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
  EcUdc item;
  // ignore parts just collect all info from items
  EcUdc items = ecudc_node (node, "items");
  if (isNotAssigned (items))
  {
    return; 
  }
  // iterrate through all items to get info
  for (item = ecudc_next (items, &cursor); isAssigned (item); item = ecudc_next (items, &cursor))
  {
    // variables
    EcUdc dbcolumn;
    EcUdc val;

    val = ecudc_node (item, ".val");
    if (isNotAssigned (val))
    {
      continue;
    }
    dbcolumn = ecudc_node (val, ".dbcolumn");
    if (isNotAssigned (dbcolumn))
    {
      continue;
    }
    adbl_query_addColumn(query, ecudc_asString(dbcolumn), 0);
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

    EcUdc items = ecudc_create (ENTC_UDC_NODE, "");

    for (c = eclist_first(query->columns); c != eclist_end(query->columns); c = eclist_next(c), colno++)
    {
      AdblQueryColumn* qc = eclist_data(c);      
      ecudc_add_asString (items, qc->column, adbl_dbcursor_data(cursor, colno));          
    }
    
    ecudc_add (values, &items);
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_dbquery (EcUdc node, EcUdc parts, ulong_t dbmin, AdboContext context, AdblSession session, AdblQuery* query)
{
  AdblSecurity adblsec;
  AdblCursor* cursor;
  int ret;
  
  adbo_node_dbquery_columns (node, query);
  // execute sql query
  cursor = adbl_dbquery (session, query, &adblsec);
  
  ret = adbo_node_dbquery_cursor (node, parts, dbmin, context, cursor, query);
  
  adbl_dbcursor_release (&cursor);
  
  return ret;  
}

//----------------------------------------------------------------------------------------

EcUdc adbo_node_values (EcUdc node)
{
  const EcString dbtable = ecudc_get_asString(node, ".dbtable", NULL);
  if (isNotAssigned (dbtable))
  {
    return NULL;
  }  
  
  return ecudc_node(node, dbtable);
}

//----------------------------------------------------------------------------------------

int adbo_node_fetch (EcUdc node, EcUdc data, AdboContext context)
{
  // variables
  const EcString dbsource;
  AdblSession dbsession;
  EcUdc values;
  ulong_t dbmin;
  AdblConstraint* constraints;
  AdblQuery* query;

  int ret = TRUE;
  
  const EcString dbtable = ecudc_get_asString(node, ".dbtable", NULL);
  if (isNotAssigned (dbtable))
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{fetch} .dbtable not defined");
    return FALSE;
  }
  
  ecudc_del (node, dbtable);
  
  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_logformat (context->logger, LL_ERROR, "ADBO", "{fetch} can't connect to database '%s'", dbsource);
    return FALSE;
  }
  
  values = ecudc_create (ENTC_UDC_LIST, dbtable);
  
  dbmin = ecudc_get_asL(node, ".dbmin", 1);
  
  
  query = adbl_query_new ();
  // add some default stuff
  eclogger_log(context->logger, LL_TRACE, "ADBO", "{request} prepare query");
  
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
  
  eclogger_log(context->logger, LL_TRACE, "ADBO", "{request} query done");
  
  ecudc_add (node, &values);
  
  return ret;  
}

//----------------------------------------------------------------------------------------

void adbo_node_insert_values (AdboContext context, EcUdc items, EcUdc update_item, EcUdc values, AdblAttributes* attrs)
{
  EcUdc item;
  void* cursor = NULL;
  
  for (item = ecudc_next (items, &cursor); isAssigned (item); item = ecudc_next (items, &cursor))
  {
    // variables
    const EcString dbcolumn;
    const EcString update_value;
    EcUdc val;

    val = ecudc_node (item, ".val");
    if (isNotAssigned (val))
    {
      eclogger_log (context->logger, LL_WARN, "ADBO", "{insert} item in items is not a value");
      continue;
    }
    
    dbcolumn = ecudc_get_asString(val, ".dbcolumn", NULL);
    if (isNotAssigned (dbcolumn))
    {
      eclogger_log (context->logger, LL_WARN, "ADBO", "{insert} value in items without dbcolumn");
      continue;
    }

    update_value = ecudc_get_asString(update_item, dbcolumn, NULL);
    if (isNotAssigned (update_value))
    {
      eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{insert} item '%s' has no value and will not be insert", dbcolumn);    
      continue;
    }
 
    adbl_attrs_addChar(attrs, dbcolumn, update_value);
    
    ecudc_add_asString(values, dbcolumn, update_value);
  }
}

//----------------------------------------------------------------------------------------

int adbo_node_insert (EcUdc node, AdboContext context, AdblSession dbsession, const EcString dbtable, EcUdc update_data, EcUdc items, AdblConstraint* constraint)
{
  void* cursor = NULL;

  EcUdc item_update;
  EcUdc values_list;

  int ret = TRUE;

  ecudc_del (node, dbtable);
  values_list = ecudc_create (ENTC_UDC_LIST, dbtable);
  
  // iterate through all new values
  for (item_update = ecudc_next (update_data, &cursor); isAssigned (item_update); item_update = ecudc_next (update_data, &cursor))
  {
    // has the item a primary key definition
    if (adbo_node_primary (node, item_update, context, NULL))
    {
      // good now check if it is in the original
      eclogger_log (context->logger, LL_WARN, "ADBO", "{update} multi update not implemented yet");
      ret = FALSE;
    } 
    else
    {
      EcUdc values = ecudc_create(ENTC_UDC_NODE, "");
      
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

      adbo_node_primary_sequence (node, dbsession, dbtable, values, attrs);      
      // all additional values
      adbo_node_insert_values (context, items, item_update, values, attrs);
      
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

int adbo_node_update_single (AdboContext context, AdblSession dbsession, const EcString dbtable, EcUdc update_data, EcUdc items, EcUdc values, AdblConstraint* constraint)
{
  int ret = TRUE;
  EcUdc item;
  void* cursor = NULL;
  
  AdblAttributes* attrs = adbl_attrs_new ();
    
  for (item = ecudc_next (items, &cursor); isAssigned (item); item = ecudc_next (items, &cursor))
  {
    // variables
    const EcString dbcolumn;
    EcUdc val;

    val = ecudc_node (item, ".val");
    if (isNotAssigned (val))
    {
      eclogger_log (context->logger, LL_WARN, "ADBO", "{update} item in items is not a value");
      continue;
    }
    
    dbcolumn = ecudc_get_asString(val, ".dbcolumn", NULL);
    if (isNotAssigned (dbcolumn))
    {
      eclogger_log (context->logger, LL_WARN, "ADBO", "{update} value in items without dbcolumn");
      continue;
    }
    
    {
      // we grab just the first item from the array
      void* cursor_value = NULL;
      void* cursor_update = NULL;
      const EcString update_value;
      
      EcUdc item_value = ecudc_next (values, &cursor_value);
      EcUdc item_update = ecudc_next (update_data, &cursor_update);
      
      if (isNotAssigned (item_value))
      {
        eclogger_log (context->logger, LL_WARN, "ADBO", "{update} array has no fetched values");
        continue;
      }

      if (isNotAssigned (item_update))
      {
        eclogger_log (context->logger, LL_WARN, "ADBO", "{update} update array is empty");
        continue;
      }
      
      // now we retrieve the item from original fetched and want to update
      update_value = ecudc_get_asString(item_update, dbcolumn, NULL);
      if (isNotAssigned (update_value))
      {
        eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{update} item '%s' has no value and will not be updated", dbcolumn);    
        continue;
      }
      
      //const EcString fetched_value = ecudc_get_asString(item_value, dbcolumn, NULL);
      
      //if (ecstr_equal(fetched_value, update_value))
      {
        // ignore if they are anyway the same
        //continue;
      }
      
      adbl_attrs_addChar(attrs, dbcolumn, update_value);      
    }    
  }  

  // do we have something to update?
  if (!adbl_attrs_empty(attrs))
  {
    AdblSecurity adblsec;
    AdblUpdate* update = adbl_update_new ();
    // create the statement
    adbl_update_setTable(update, dbtable);
    adbl_update_setConstraint(update, constraint);
    adbl_update_setAttributes(update, attrs);
    
    ret = adbl_dbupdate (dbsession, update, &adblsec);
    
    adbl_update_delete(&update);
  }
  else
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{update} nothing to update");    
  }
  
  adbl_attrs_delete(&attrs);
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update_state (EcUdc node, EcUdc filter, AdboContext context, AdblSession dbsession, const EcString dbtable, EcUdc update_data, EcUdc items, EcUdc values)
{
  int ret = TRUE;
  
  AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);

  if (adbo_node_primary (node, filter, context, constraint))
  {
    // explicitely check the values
    if (isAssigned (values))
    {
      // there is a primary key given: good it makes things easier
      ret = adbo_node_update_single (context, dbsession, dbtable, update_data, items, values, constraint);            
    }
    else
    {
      eclogger_log (context->logger, LL_ERROR, "ADBO", "{update} values must be assigned for update");
      ret = FALSE;
    }
  }
  else if (adbo_node_foreign (node, filter, context, constraint))
  {
    // there is a primary key given: good it makes things easier
    ret = adbo_node_insert (node, context, dbsession, dbtable, update_data, items, constraint);            
  }
  else
  {
    ret = adbo_node_insert (node, context, dbsession, dbtable, update_data, items, NULL);            
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
  EcUdc items;
  EcUdc values;

  int ret = FALSE;
  
  const EcString dbtable = ecudc_get_asString(node, ".dbtable", NULL);
  if (isNotAssigned (dbtable))
  {
    eclogger_log (context->logger, LL_ERROR, "ADBO", "{fetch} .dbtable not defined");
    return FALSE;
  }
  
  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_logformat (context->logger, LL_ERROR, "ADBO", "{update} can't connect to database '%s'", dbsource);
    return FALSE;
  }
  
  if (isNotAssigned (data))
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{update} no data given");
    return FALSE;
  }
  
  // check if the data fits the structure
  update_data = ecudc_node(data, dbtable);
  if (isNotAssigned (update_data))
  {
    eclogger_logformat (context->logger, LL_WARN, "ADBO", "{update} missing data for table '%s'", dbtable);
    return FALSE;    
  }
  
  if (ecudc_type(update_data) != ENTC_UDC_LIST)
  {
    eclogger_logformat (context->logger, LL_WARN, "ADBO", "{update} data for '%s' is not a list object", dbtable);
    return FALSE;    
  }
  
  items = ecudc_node(node, "items");
  if (isNotAssigned (items))
  {
    eclogger_logformat (context->logger, LL_WARN, "ADBO", "{update} no items found for '%s'", dbtable);
    return FALSE;    
  }

  // values can be empty
  values = ecudc_node(node, dbtable);  
  
  if (withTransaction)
  {
    adbl_dbbegin (dbsession);
  }
  
  ret = adbo_node_update_state (node, filter, context, dbsession, dbtable, update_data, items, values);
    
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
  
  const EcString dbtable = ecudc_get_asString(node, ".dbtable", NULL);
  if (isNotAssigned (dbtable))
  {
    eclogger_log (context->logger, LL_ERROR, "ADBO", "{fetch} .dbtable not defined");
    return FALSE;
  }
  
  dbsource = ecudc_get_asString(node, ".dbsource", "default");  
  dbsession = adbl_openSession (context->adblm, dbsource);
  // delete all previous entries
  if (isNotAssigned (dbsession))
  {
    eclogger_logformat (context->logger, LL_ERROR, "ADBO", "{update} can't connect to database '%s'", dbsource);
    return FALSE;
  }
      
  if (withTransaction)
  {
    adbl_dbbegin (dbsession);
  }
  
  {
    AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
    
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
    else
    {
      eclogger_log (context->logger, LL_ERROR, "ADBO", "{delete} needs valid primary keys set");
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
