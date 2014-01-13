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

#include "adbo_node.h"

#include "adbo_object.h"
#include "adbo_schema.h"
#include "adbo_container.h"

#include <types/eclist.h>
#include <adbl.h>

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
  
};

//----------------------------------------------------------------------------------------

void adbo_node_dbkeys_del (EcList* pself)
{
  EcListNode node;
  EcList self = *pself;
  
  for (node = eclist_first (self); node != eclist_end (self); node = eclist_next (node))
  {
    AdboValue value = (AdboValue)eclist_data (node);
    adbo_value_del (&value);    
  }
  
  eclist_delete (pself);  
}

//----------------------------------------------------------------------------------------

EcList adbo_node_dbkeys_clone (const EcList oself)
{
  EcListNode node;
  EcList self = eclist_new ();
  
  for (node = eclist_first (oself); node != eclist_end (oself); node = eclist_next (node))
  {
    eclist_append (self, adbo_value_clone ((AdboValue)eclist_data (node)));
  }
    
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_node_dbkeys_parse (AdboNode self, EcList list, const EcString tag, EcXMLStream xmlstream, EcLogger logger)
{
  ENTC_XMLSTREAM_BEGIN
  
  if (ecxmlstream_isBegin (xmlstream, "value"))
  {
    AdboValue value = adbo_value_newFromXml (xmlstream);
    
    eclist_append(list, value);
    
    eclogger_logformat (logger, LL_TRACE, "ADBO", "{parse} add to %s: '%s'", tag, adbo_value_getDBColumn(value));
    
    if (isNotAssigned (self->parts) && !adbo_value_hasLocalLink (value))
    {
      // create the list for parts
      self->parts = eclist_new();      
    }
  }
  
  ENTC_XMLSTREAM_END (tag)
}

//----------------------------------------------------------------------------------------

int adbo_node_dbkeys_has (EcList self, AdboValue value)
{
  EcListNode node;
  for (node = eclist_first (self); node != eclist_end (self); node = eclist_next (node))
  {
    if (value == (AdboValue)eclist_data (node))
    {
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------

int adbo_value_constraint_add (AdboValue value, AdboContainer container, AdblConstraint* constraint, EcLogger logger, int logall)
{
  const EcString val1 = adbo_value_getDBColumn (value);
  const EcString val2 = adbo_value_cget (value, container);
  if (ecstr_valid(val1) && ecstr_valid(val2))
  {
    eclogger_logformat(logger, LL_TRACE, "ADBO", "{attrs} values found '%s' '%s'", val1, val2);
    adbl_constraint_addChar (constraint, val1, QUOMADBL_CONSTRAINT_EQUAL, val2);
    return TRUE;
  }
  else if (logall)
  {
    eclogger_logformat(logger, LL_WARN, "ADBO", "{constraints} no valid values found '%s' '%s'", val1, val2);    
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------

int adbo_value_attrs_add (AdboValue value, AdboContainer container, AdblAttributes* attrs, EcLogger logger)
{
  const EcString val1 = adbo_value_getDBColumn (value);
  const EcString val2 = adbo_value_cget (value, container);
  if (ecstr_valid(val1) && ecstr_valid(val2))
  {
    eclogger_logformat(logger, LL_TRACE, "ADBO", "{attrs} values found '%s' '%s'", val1, val2);
    adbl_attrs_addChar (attrs, val1, val2);
    return TRUE;
  }
  else
  {
    eclogger_logformat(logger, LL_WARN, "ADBO", "{attrs} no valid values found '%s' '%s'", val1, val2);
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------

int adbo_node_dbkeys_constraints (EcList self, AdboContainer container, AdblConstraint* constraint, EcLogger logger, int logall)
{
  int ret = TRUE;
  EcListNode node;
  for (node = eclist_first (self); node != eclist_end (self); node = eclist_next (node))
  {
    ret = ret && adbo_value_constraint_add ((AdboValue)eclist_data (node), container, constraint, logger, logall);
  }
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_checkLocal (AdboValue value, AdboContainer container, AdblAttributes* attrs, AdboContext context)
{
  const EcString val1;
  const EcString val2;
  AdboValue dest_value;
  AdboObject dest_obj;
  
  dest_obj = adbo_value_cobject (value, container);
  
  if (isNotAssigned (dest_obj))
  {
    return FALSE;
  }
  
  dest_value = adbo_getValue (dest_obj);
  
  if (isNotAssigned (dest_value))
  {
    return FALSE;
  }
  
  val1 = adbo_value_getDBColumn (dest_value);
  if (!ecstr_valid(val1))
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{attrs} dbcolumn not set");
    return FALSE;
  }
  
  val2 = adbo_value_getData (dest_value);
  if (ecstr_valid(val2))
  {
    // we got a value
    eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{attrs} values found '%s' '%s'", val1, val2);
    adbl_attrs_addChar (attrs, val1, val2);
    return TRUE;      
  }
  
  eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{attrs} pre-update for foreign key '%s'", val1);

  // trigger the update process of the destination object
  if (!adbo_object_update (dest_obj, context, FALSE))
  {
    return FALSE;
  }
  
  // now check again
  val2 = adbo_value_getData (dest_value);
  if (ecstr_valid(val2))
  {
    // we got a value
    eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{attrs} values found '%s' '%s'", val1, val2);
    adbl_attrs_addChar (attrs, val1, val2);
    return TRUE;      
  }
  
  return FALSE;
}

//----------------------------------------------------------------------------------------

int adbo_node_dbkeys_attrs (EcList self, AdboContainer container, AdblAttributes* attrs, AdboContext context)
{
  EcListNode node;
  for (node = eclist_first (self); node != eclist_end (self); node = eclist_next (node))
  {
    AdboValue value = (AdboValue)eclist_data (node);
    
    if (adbo_value_hasLocalLink (value))
    {
      // check first local link
      if (!adbo_node_checkLocal (value, container, attrs, context))
      {
        return FALSE;
      }
    }
    else
    {
      if (!adbo_value_attrs_add (value, container, attrs, context->logger))
      {
        // the value is not valid
        return FALSE;
      }      
    }
  }
  return TRUE;
}

//########################################################################################

AdboNodePart adbo_nodepart_new (AdboContainer parent)
{
  AdboNodePart self = ENTC_NEW (struct AdboNodePart_s);
  
  self->container = adbo_container_new (ADBO_CONTAINER_NODE, parent);
  self->state = ADBO_STATE_NONE;
  
  self->primary_keys = eclist_new ();

  return self;
}

//----------------------------------------------------------------------------------------

void adbo_nodepart_del (AdboNodePart* pself)
{
  AdboNodePart self = *pself;
  
  adbo_container_del (&(self->container));

  eclist_delete(&(self->primary_keys));
  
  ENTC_DEL (pself, struct AdboNodePart_s);
}

//----------------------------------------------------------------------------------------

void adbo_nodepart_checkPrimaryKeys (void* ptr1, void* ptr2, void* data1, void* data2)
{
  if (adbo_node_dbkeys_has ((EcList)ptr1, data1))
  {
    // obviously this is a primary key
    eclist_append ((EcList)ptr2, data2);
  }  
}

//----------------------------------------------------------------------------------------

AdboNodePart adbo_nodepart_clone (const AdboNodePart oself, AdboContainer parent)
{
  AdboNodePart self = ENTC_NEW (struct AdboNodePart_s);
  
  self->primary_keys = eclist_new ();

  self->container = adbo_container_clone (oself->container, parent, adbo_nodepart_checkPrimaryKeys, oself->primary_keys, self->primary_keys);
  
  self->state = oself->state;
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_nodepart_setValues (AdboNodePart self, AdblCursor* cursor, int state, EcLogger logger)
{
  adbo_container_set (self->container, cursor, logger);

  self->state = state;
}

//----------------------------------------------------------------------------------------

void adbo_nodepart_dump (AdboNodePart self, int depth, int le, int partno, EcBuffer b2, EcLogger logger)
{
  unsigned char* buffer = b2->buffer;
  
  uint_t pos = depth * 2;
  
  buffer [pos] = le ? '\\' : '[';        
  buffer [pos + 1] = '_';
  buffer [pos + 2] = '_';
  buffer [pos + 3] = 0;
  
  eclogger_logformat(logger, LL_TRACE, "ADBO", "%s dataset [%u] state: %s"
                     , ecstr_get (b2)
                     , partno
                     , adbo_dump_state (self->state)
                     );    
  
  buffer [pos] = le ? ' ' : '|';
  buffer [pos + 1] = ' ';
  buffer [pos + 2] = 0;      
  
  adbo_container_dump (self->container, depth + 1, b2, logger);      
  
}

//########################################################################################

AdboNode adbo_node_new1 (AdboObject obj, AdboContext context, AdboContainer parent, EcXMLStream xmlstream)
{
  AdboNode self = ENTC_NEW (struct AdboNode_s);

  self->spart = adbo_nodepart_new (parent);
  
  self->dbsource = ecstr_copy ("default");
  self->dbtable = ecstr_init ();
  
  self->foreign_keys = eclist_new ();

  self->max = 0;
  self->min = 0;

  self->parts = NULL;

  if (isAssigned (xmlstream))
  {
    ecstr_replace (&(self->dbtable), ecxmlstream_nodeAttribute (xmlstream, "dbtable"));

    eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{parse} node '%s'", self->dbtable); 
    
    ENTC_XMLSTREAM_BEGIN
    
    if (ecxmlstream_isBegin (xmlstream, "objects"))
    {
      const EcString max = ecxmlstream_nodeAttribute (xmlstream, "max");
      const EcString min = ecxmlstream_nodeAttribute (xmlstream, "min");
      if (ecstr_valid (max))
      {
        self->max = atoi(max);
      }
      if (ecstr_valid (min))
      {
        self->min = atoi(min);
      }

      adbo_objects_fromXml (self->spart->container, context, xmlstream, "objects");
    }
    else if (ecxmlstream_isBegin (xmlstream, "primary_keys"))
    {
      AdboObject item = adbo_object_new1 (self->spart->container, context, ADBO_OBJECT_ITEM, xmlstream, "primary_keys");
      
      adbo_container_add (self->spart->container, item);
      
      eclist_append (self->spart->primary_keys, adbo_getValue (item));
    }
    else if (ecxmlstream_isBegin (xmlstream, "foreign_keys"))
    {
      adbo_node_dbkeys_parse (self, self->foreign_keys, "foreign_keys", xmlstream, context->logger);
    }
    else if (ecxmlstream_isBegin (xmlstream, "value"))
    {
      adbo_setValue (obj, adbo_value_newFromXml (xmlstream));
    }
    else if (ecxmlstream_isBegin (xmlstream, "data"))
    {
      
    }
    
    ENTC_XMLSTREAM_END ("node")
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

AdboNode adbo_node_new2 (AdboObject obj, AdboContext context, AdboContainer parent, AdblTable* table_info, const EcString origin)
{
  EcListNode node;
  AdboNode self = ENTC_NEW (struct AdboNode_s);

  self->spart = adbo_nodepart_new (parent);
  
  self->dbsource = ecstr_copy ("default");
  self->dbtable = ecstr_copy (table_info->name);
  
  self->foreign_keys = eclist_new ();
  
  self->max = 0;
  self->min = 0;
  
  self->parts = NULL;
  
  // primary keys
  for (node = eclist_first (table_info->primary_keys); node != eclist_end (table_info->primary_keys); node = eclist_next (node))
  {
    AdboValue value = adbo_value_new (eclist_data(node), NULL, NULL);
    
    AdboObject item = adbo_object_new2 (self->spart->container, context, ADBO_OBJECT_ITEM, NULL, origin, value);
    
    adbo_container_add (self->spart->container, item);
    
    eclist_append (self->spart->primary_keys, adbo_getValue (item));    
  }
  
  // foreign keys
  for (node = eclist_first (table_info->foreign_keys); node != eclist_end (table_info->foreign_keys); node = eclist_next (node))
  {
    
    AdblForeignKeyConstraint* fk = eclist_data(node);
    
    if (ecstr_equal(origin, fk->table))
    {
      EcString link = ecstr_cat2("../", fk->reference);
      
      AdboValue value = adbo_value_new (fk->column_name, NULL, link);

      eclist_append(self->foreign_keys, value);
      
      ecstr_delete(&link);
    }
    else
    {
      AdboValue value = adbo_value_new (fk->column_name, NULL, fk->table);
      
      eclist_append(self->foreign_keys, value);
      
      eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{fromdb} add to %s: '%s'", table_info->name, adbo_value_getDBColumn(value));
      
      //AdboObject obj = adbo_schema_get (context->schema, context, self->spart->container, fk->table, table_info->name);
      //if (isAssigned (obj))
      //{
      //  adbo_container_add (self->spart->container, obj);
      //}
    }
    
    
    
  }

  {
    EcList objects = eclist_new ();
    // fill a list with all objects which have foreign key contraints to this table
    adbo_schema_ref (context->schema, context, self->spart->container, table_info->name, objects);
  
    for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
    {
      
      adbo_container_add (self->spart->container, eclist_data(node));
    }
    
    eclist_delete (&objects);
  }
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_node_del (AdboNode* pself)
{
  AdboNode self = *pself;

  ecstr_delete (&(self->dbsource));
  ecstr_delete (&(self->dbtable));
  
  adbo_nodepart_del (&(self->spart));

  adbo_node_dbkeys_del (&(self->foreign_keys));
    
  ENTC_DEL (pself, struct AdboNode_s);
}

//----------------------------------------------------------------------------------------

AdboNode adbo_node_clone (const AdboNode oself, AdboContainer parent)
{

  AdboNode self = ENTC_NEW (struct AdboNode_s);
  
  self->spart = adbo_nodepart_clone (oself->spart, parent);
  
  self->dbsource = ecstr_copy (oself->dbsource);
  self->dbtable = ecstr_copy (oself->dbtable);

  self->foreign_keys = adbo_node_dbkeys_clone (oself->foreign_keys);
  
  if (isAssigned (oself->parts))
  {
    self->parts = eclist_new();
  }
  else
  {
    self->parts = NULL;
  }
  
  self->min = oself->min;
  self->max = oself->max;
  
  return self;
}

//----------------------------------------------------------------------------------------

AdblConstraint* adbo_node_request_constraints (AdboNode self, AdboNodePart part, int checkAll, EcLogger logger, int logall)
{
  AdblConstraint* constraint = adbl_constraint_new (QUOMADBL_CONSTRAINT_AND);
  // check primary keys and foreign keys if exists and add them
  adbo_node_dbkeys_constraints (part->primary_keys, part->container, constraint, logger, (!checkAll) && logall);
  if (adbl_constraint_empty (constraint) && checkAll)
  {
    adbo_node_dbkeys_constraints (self->foreign_keys, part->container, constraint, logger, logall);
  }
  
  if (adbl_constraint_empty (constraint))
  {
    adbl_constraint_delete (&constraint);
  }
  
  return constraint;
}

//----------------------------------------------------------------------------------------

AdblAttributes* adbo_node_request_attrs (AdboNode self, AdboNodePart part, AdboContext context)
{
  AdblAttributes* attrs = adbl_attrs_new ();
  // check primary keys and foreign keys if exists and add them
  if (!adbo_node_dbkeys_attrs (part->primary_keys, part->container, attrs, context))
  {
    adbl_attrs_delete (&attrs);
    return attrs;  
  }
  
  if (!adbl_attrs_empty (attrs))
  {
    if (!adbo_node_dbkeys_attrs (self->foreign_keys, part->container, attrs, context))
    {
      adbl_attrs_delete (&attrs);
      return attrs;          
    }
  }
    
  if (adbl_attrs_empty (attrs))
  {
    adbl_attrs_delete (&attrs);
  }
  
  return attrs;
}

//----------------------------------------------------------------------------------------

int adbo_node_request_query (AdboNode self, AdboContext context, AdblSession session, AdblQuery* query)
{
  AdblSecurity adblsec;
  AdblCursor* cursor;
  int ret = TRUE;
  
  // ***** retrieve all columns, construct the sql query *****
  adbo_container_query (self->spart->container, query);  
  // execute sql query
  cursor = adbl_dbquery (session, query, &adblsec);
  
  if (isAssigned (self->parts))
  {
    uint_t count = 0;
    uint_t i;
    // multi mode
    while (adbl_dbcursor_next (cursor)) 
    {
      // create a new part
      AdboNodePart part = adbo_nodepart_clone (self->spart, adbo_container_parent (self->spart->container));
      
      eclist_append (self->parts, part);
      
      adbo_nodepart_setValues (part, cursor, ADBO_STATE_ORIGINAL, context->logger);  
      
      count++;

      eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{request} added original part %u", count); 
    }
    
    if (self->min - count > 0)
    {
      eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{request} no full dataset found just %u from %u", count, self->min); 
    }
    
    for (i = count; i < self->min; i++)
    {
      // create a new empty part
      AdboNodePart part = adbo_nodepart_clone (self->spart, adbo_container_parent (self->spart->container));
      
      eclist_append (self->parts, part);

      eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{request} added empty part %u", i + 1); 
    }   
    
    {
      EcListNode node;
      // iterate through all parts and request them
      for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
      {
        AdboNodePart part = eclist_data (node);
        ret = ret && adbo_container_request (part->container, context);
      }
    }    
    return ret;
  }
  else
  {
    // single mode
    if (adbl_dbcursor_next (cursor))
    {
      // ***** fill with data *****
      adbo_nodepart_setValues (self->spart, cursor, ADBO_STATE_ORIGINAL, context->logger);   
      if (adbl_dbcursor_next (cursor))
      {
        eclogger_log(context->logger, LL_ERROR, "ADBO", "{request} query returned multiple dataset");
        ret = FALSE;
      }
      return ret && adbo_container_request (self->spart->container, context);
    }
    else
    {
      eclogger_log(context->logger, LL_TRACE, "ADBO", "{request} query returned no dataset");
      self->spart->state = ADBO_STATE_INSERTED;
      return ret;
    }    
  }
}

//----------------------------------------------------------------------------------------

int adbo_node_request (AdboNode self, AdboContext context)
{
  int ret = FALSE;
  
  AdblSession dbsession = adbl_openSession (context->adblm, self->dbsource);
  if (isAssigned (dbsession))
  {
    AdblConstraint* constraints;
    AdblQuery* query = adbl_query_new ();
    // add some default stuff
    eclogger_log(context->logger, LL_TRACE, "ADBO", "{request} prepare query");
    
    // ***** check constraints *****
    // we can use spart here, because constrainst are the same for all parts
    constraints = adbo_node_request_constraints (self, self->spart, isAssigned(self->parts), context->logger, FALSE);
    if (isAssigned (constraints))
    {
      adbl_query_setConstraint (query, constraints);
      
      adbl_query_setTable (query, self->dbtable);
      // execute in extra method to avoid memory leaks in query and dbsession
      ret = adbo_node_request_query (self, context, dbsession, query);
      
      adbl_constraint_delete (&constraints);
    }
    else
    {
      if (isAssigned (self->parts))
      {
        uint_t i;
        for (i = 0; i < self->min; i++)
        {
          // create a new empty part
          AdboNodePart part = adbo_nodepart_clone (self->spart, adbo_container_parent (self->spart->container));
          
          eclist_append (self->parts, part);

          eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{request} added empty part %u", i + 1); 
        } 
        
        {
          EcListNode node;
          // iterate through all parts and request them
          for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
          {
            AdboNodePart part = eclist_data (node);
            ret = ret && adbo_container_request (part->container, context);
          }
        }        
      }
      else
      {
        eclogger_logformat(context->logger, LL_WARN, "ADBO", "{request} no valid primary or foreign keys found for '%s'", self->dbtable);         
      }
    }
    adbl_query_delete (&query);
    
    adbl_closeSession (&dbsession);
        
    eclogger_log(context->logger, LL_TRACE, "ADBO", "{request} query done");
  }  
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update_query (AdboNode self, AdboNodePart part, AdboContext context, AdblSession session, AdblUpdate* update)
{
  AdblSecurity adblsec;
  int ret = TRUE;
  
  AdblAttributes* attrs;
  
  if (!adbo_container_update (part->container, context))
  {
    return FALSE;
  }
  
  attrs = adbl_attrs_new ();
  // ***** retrieve all columns, construct the sql query *****
  adbo_container_attrs (part->container, attrs);
  
  if (!adbl_attrs_empty (attrs))
  {
    adbl_update_setAttributes (update, attrs);
    
    ret = adbl_dbupdate (session, update, &adblsec);
  }
  
  adbl_attrs_delete (&attrs);    
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update_update (AdboNode self, AdboNodePart part, AdboContext context, AdblSession dbsession)
{
  int ret = FALSE;
  AdblUpdate* update = adbl_update_new ();
  AdblConstraint* constraints;  
  // ***** check constraints *****
  constraints = adbo_node_request_constraints (self, part, isAssigned(self->parts), context->logger, TRUE);
  if (isAssigned (constraints))
  {
    adbl_update_setConstraint (update, constraints);
    adbl_update_setTable (update, self->dbtable);

    ret = adbo_node_update_query (self, part, context, dbsession, update);

    adbl_constraint_delete (&constraints);
  }
  else
  {
    eclogger_log(context->logger, LL_WARN, "ADBO", "{update} no valid primary or foreign keys found"); 
  }

  adbl_update_delete (&update);

  eclogger_log(context->logger, LL_TRACE, "ADBO", "{update} update done");

  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update_insert (AdboNode self, AdboNodePart part, AdboContext context, AdblSession dbsession)
{
  int ret = FALSE;
  AdblSecurity adblsec;
  
  eclogger_log(context->logger, LL_DEBUG, "ADBO", "{insert} begin"); 

  {
    AdblAttributes* attrs;  
    // ***** check constraints *****
    attrs = adbo_node_request_attrs (self, part, context);
    if (isAssigned (attrs))
    {
      AdblInsert* insert = adbl_insert_new ();
          
      // check here if we have all primary keys with values
      // and fill them with cool new ones
      if (isAssigned (self->parts))
      {

      }

      adbl_insert_setAttributes (insert, attrs);
      adbl_insert_setTable (insert, self->dbtable);
          
      ret = adbl_dbinsert (dbsession, insert, &adblsec);                        

      adbl_insert_delete (&insert);

      adbl_attrs_delete (&attrs);
    }
    else
    {
      eclogger_log(context->logger, LL_WARN, "ADBO", "{insert} no valid primary or foreign keys found"); 
    }
  }
  
  ret = adbo_container_update (part->container, context);

  eclogger_log(context->logger, LL_TRACE, "ADBO", "{insert} update done");

  return ret;
}

//----------------------------------------------------------------------------------------

EcString adbo_node_update_sequence (AdboNode self, AdboContext context, AdblSession dbsession)
{
  AdblSequence* sequence;
  EcString seqno;
  // now we need to create a new sequence entry
  sequence = adbl_dbsequence_get (dbsession, self->dbtable);
  
  seqno = ecstr_long (adbl_sequence_next (sequence));
  
  eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{update} create new value '%s' for table '%s'", seqno, self->dbtable); 
  
  adbl_sequence_release (&sequence);
  
  return seqno;
}

//----------------------------------------------------------------------------------------

void adbo_node_create_primarykeys (AdboNode self, AdboNodePart part, AdboContext context, AdblSession dbsession)
{
  EcListNode node;
  // iterate through all objects
  for (node = eclist_first (part->primary_keys); node != eclist_end (part->primary_keys); node = eclist_next (node))
  {
    AdboValue value = adbo_value_cseek ((AdboValue)eclist_data (node), part->container);
    if (isAssigned (value))
    {
      EcString seqno;
      // check state
      if (adbo_value_getState (value) == ADBO_STATE_FIXED)
      {
        eclogger_log (context->logger, LL_WARN, "ADBO", "{primarykeys} no need for create");
        // we have some kind of data already
        // however this will not work logically it is formally correct
        return;    
      }
      // check other states
      if (adbo_value_getState (value) != ADBO_STATE_NONE)
      {
        const EcString data = adbo_value_getData (value);
        
        if (ecstr_valid(data))
        {
          eclogger_logformat (context->logger, LL_WARN, "ADBO", "{primarykeys} key is not none (%u)", adbo_value_getState (value));
          return;          
        }
      }
      
      seqno = adbo_node_update_sequence (self, context, dbsession);
      
      adbo_value_set (value, seqno, ADBO_STATE_INSERTED);
      
      ecstr_delete (&seqno);
    }
  }
}

//----------------------------------------------------------------------------------------

int adbo_node_update_state (AdboNode self, AdboNodePart part, AdboContext context, AdblSession dbsession)
{
  int ret = FALSE;
  int state = part->state;
  switch (state)
  {
    case ADBO_STATE_NONE:
    {
      eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{update} update none for table '%s'", self->dbtable); 
      // this object node was not requested and is totally virtual
      // for this case we need to create new primary keys
      adbo_node_create_primarykeys (self, part, context, dbsession);
      // after creating the primary key we can continue with inserting
      ret = adbo_node_update_insert (self, part, context, dbsession);
      if (ret)
      {
        part->state = ADBO_STATE_ORIGINAL;
      }
    }
    break;
    case ADBO_STATE_ORIGINAL:
    {
      eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{update} update original for table '%s'", self->dbtable); 
      ret = adbo_node_update_update (self, part, context, dbsession);
    }
    break;
    case ADBO_STATE_INSERTED:
    {
      eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{update} update insert for table '%s'", self->dbtable); 
      ret = adbo_node_update_insert (self, part, context, dbsession);
      if (ret)
      {
        part->state = ADBO_STATE_ORIGINAL;
      }
    }
    break;
  }
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_update (AdboNode self, AdboContext context, int withTransaction)
{
  int ret = FALSE;
  AdblSession dbsession = adbl_openSession (context->adblm, self->dbsource);
  if (isAssigned (dbsession))
  {
    if (withTransaction)
    {
      adbl_dbbegin (dbsession);
    }
    
    if (isAssigned (self->parts))
    {
      // mutli mode  
      EcListNode node;
      ret = TRUE;
      for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
      {
        ret = ret && adbo_node_update_state (self, eclist_data(node), context, dbsession);
      }
    }
    else
    {
      // single mode
      ret = adbo_node_update_state (self, self->spart, context, dbsession);
    }
    
    if (withTransaction)
    {
      // commit / rollback all changes
      adbo_node_transaction (self, ret);
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
  }
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_delete_next (AdboNode self, AdboNodePart part, AdboContext context, AdblSession dbsession)
{
  int ret = FALSE;

  AdblConstraint* constraints;
  AdblDelete* delete = adbl_delete_new ();
  // add some default stuff
  eclogger_log(context->logger, LL_TRACE, "ADBO", "{delete} prepare query");
  
  // ***** check constraints *****
  constraints = adbo_node_request_constraints (self, part, FALSE, context->logger, TRUE);
  if (isAssigned (constraints))
  {
    AdblSecurity adblsec;
    
    adbl_delete_setConstraint (delete, constraints);
    
    adbl_delete_setTable (delete, self->dbtable);
    // execute in extra method to avoid memory leaks in query and dbsession
    ret = adbl_dbdelete (dbsession, delete, &adblsec);
    
    adbl_constraint_delete (&constraints);
    
    ret = ret && adbo_container_delete (part->container, context);
  }
  else
  {
    eclogger_log(context->logger, LL_WARN, "ADBO", "{delete} no valid primary or foreign keys found"); 
  }
  adbl_delete_delete (&delete);      
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_node_delete (AdboNode self, AdboContext context, int withTransaction)
{
  int ret = FALSE;
  AdblSession dbsession = adbl_openSession (context->adblm, self->dbsource);
  if (isAssigned (dbsession))
  {
    if (withTransaction)
    {
      adbl_dbbegin (dbsession);
    }
    
    if (isAssigned (self->parts))
    {
      // mutli mode  
      EcListNode node;
      ret = TRUE;
      for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
      {
        ret = ret && adbo_node_delete_next (self, eclist_data(node), context, dbsession);
      }
    }
    else
    {
      // single mode
      ret = adbo_node_delete_next (self, self->spart, context, dbsession);
    }
    
    if (withTransaction)
    {
      // commit / rollback all changes
      adbo_node_transaction (self, ret);
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
  }
  return ret;  
}

//----------------------------------------------------------------------------------------

void adbo_node_transaction (AdboNode self, int state)
{
  if (isAssigned (self->parts))
  {
    EcListNode node;
    for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
    {
      AdboNodePart part = eclist_data(node);
      
      adbo_container_transaction (part->container, state);
    }    
  }
  else
  {
    adbo_container_transaction (self->spart->container, state);    
  }
}

//----------------------------------------------------------------------------------------
                                             
AdboObject adbo_node_at (AdboNode self, const EcString link)
{
  return adbo_container_at (self->spart->container, link);  
}

//----------------------------------------------------------------------------------------

void adbo_node_strHelper (AdboNode self, EcStream stream)
{
  if (isAssigned (self->parts))
  {
    // mutli mode  
    EcListNode node;

    ecstream_appendc (stream, '[');
    for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
    {
      AdboNodePart part = eclist_data (node);
      
      if (node != eclist_first (self->parts))
      {
        ecstream_appendc (stream, ',');
      }
      adbo_container_str (part->container, stream);
    }
    ecstream_appendc (stream, ']');
  }
  else
  {
    adbo_container_str (self->spart->container, stream);
  }
}

//----------------------------------------------------------------------------------------

void adbo_node_strToStream (AdboNode self, EcStream stream)
{

  if ( ecstr_valid(self->dbtable))
  {
    ecstream_appendc (stream, '"');
    ecstream_append (stream, self->dbtable);
    ecstream_append (stream, "\":");
  }
  adbo_node_strHelper (self, stream);
}

//----------------------------------------------------------------------------------------

EcString adbo_node_str (AdboNode self)
{
  EcStream stream;
  EcBuffer buffer;
  // recursively continue with the type of this object
  stream = ecstream_new ();

  adbo_node_strHelper (self, stream);
  // extract the buffer from the stream
  buffer = ecstream_trans (&stream);
  // extract string from buffer
  return ecstr_trans (&buffer);
}

//----------------------------------------------------------------------------------------

AdboObject adbo_node_get (AdboObject obj, AdboNode self, const EcString link)
{
  if (ecstr_equal (self->dbtable, link))
  {
    return obj;
  }
  else
  {
    return NULL;
  }
}

//----------------------------------------------------------------------------------------

void adbo_node_dump (AdboObject obj, AdboNode self, int depth, int le, EcBuffer b2, EcLogger logger)
{
  EcListNode node;
  
  unsigned char* buffer = b2->buffer;
  
  uint_t pos = depth * 2;
 
  uint_t state = -1000;
  
  const EcString dbcolumn = "none";
  const EcString dbvalue = "none";
  
  buffer [pos] = '|';        
  buffer [pos + 1] = '+';
  buffer [pos + 2] = '+';
  buffer [pos + 3] = 0;

  AdboValue value = (AdboValue)adbo_getValue (obj);
  if (isAssigned (value))
  {
    dbcolumn = adbo_value_getDBColumn (value);
    dbvalue = adbo_value_get (value, obj);
    state = adbo_value_getState (value);

    eclogger_logformat(logger, LL_TRACE, "ADBO", "%s value ['%s' = '%s'] state: %s"
                       , ecstr_get (b2)
                       , dbcolumn
                       , dbvalue
                       , adbo_dump_state (state)
                       );    
  }
  
  for (node = eclist_first (self->foreign_keys); node != eclist_end (self->foreign_keys); node = eclist_next (node))
  {
    AdboValue value = (AdboValue)eclist_data(node);
    if (isAssigned (value))
    {
      dbcolumn = adbo_value_getDBColumn (value);
      dbvalue = adbo_value_get (value, obj);
      state = adbo_value_getState (value);
    }
        
    eclogger_logformat(logger, LL_TRACE, "ADBO", "%s foreign key ['%s' = '%s'] state: %s"
                       , ecstr_get (b2)
                       , dbcolumn
                       , dbvalue
                       , adbo_dump_state (state)
                       );        
  }
  
  buffer [pos] = le ? '\\' : '[';        
  buffer [pos + 1] = '_';
  buffer [pos + 2] = '_';
  buffer [pos + 3] = 0;
  
  if (isAssigned (self->parts))
  {
    EcListNode node;
    uint_t counter = 0;

    eclogger_logformat(logger, LL_TRACE, "ADBO", "%s '%s' [multi] min:%u max:%u"
                       , ecstr_get (b2)
                       , self->dbtable
                       , self->min
                       , self->max
                       );    
        
    for (node = eclist_first (self->parts); node != eclist_end (self->parts); node = eclist_next (node))
    {
      counter++;
      
      buffer [pos] = le ? ' ' : '|';
      buffer [pos + 1] = ' ';
      buffer [pos + 2] = 0;      
      
      adbo_nodepart_dump ((AdboNodePart)eclist_data (node), depth + 1, eclist_next (node) == eclist_end (self->parts), counter, b2, logger);
    }
  }
  else
  {
    eclogger_logformat(logger, LL_TRACE, "ADBO", "%s '%s' [single]"
                       , ecstr_get (b2)
                       , self->dbtable
                       ); 
    
    buffer [pos] = le ? ' ' : '|';
    buffer [pos + 1] = ' ';
    buffer [pos + 2] = 0;      

    adbo_nodepart_dump (self->spart, depth + 1, le, 0, b2, logger);    
  }
}

//----------------------------------------------------------------------------------------
