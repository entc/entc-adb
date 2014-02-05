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

#include "adbo_schema.h"
#include "adbo_object.h"

#include <types/ecmap.h>

struct AdboSchema_s
{
  
  EcString dbsource;
  
  // original
  EcMap tables;
  
  // reference
  EcMap foreign_keys;
  
  EcMap schemas;
  
};

//----------------------------------------------------------------------------------------

void adbo_schema_addFKs (AdboSchema self, AdblTable* table_info)
{
  EcListNode node;
  for (node = eclist_first (table_info->foreign_keys); node != eclist_end (table_info->foreign_keys); node = eclist_next (node))
  {
    EcMapNode node2;
    EcList fkobjects;

    AdblForeignKeyConstraint* fk = eclist_data(node);

    node2 = ecmap_find(self->foreign_keys, fk->table);
    
    if (node2 == ecmap_end(self->foreign_keys))
    {
      fkobjects = eclist_new ();
    }
    else
    {
      fkobjects = ecmap_data (node2);
    }
        
    eclist_append(fkobjects, fk);
    
    ecmap_append(self->foreign_keys, fk->table, fkobjects);
  }
}

//----------------------------------------------------------------------------------------

AdboSchema adbo_schema_new (AdboContext context, const EcString dbsource)
{
  AdboSchema self = ENTC_NEW (struct AdboSchema_s);

  self->tables = ecmap_new ();
  self->foreign_keys = ecmap_new ();
  
  self->schemas = ecmap_new ();
  self->dbsource = ecstr_copy (dbsource);
    
  {
    // open session
    AdblSession dbsession = adbl_openSession (context->adblm, self->dbsource);
    if (isAssigned (dbsession))
    {
      EcListNode node;
      
      EcList tables = adbl_dbschema (dbsession);

      for (node = eclist_first (tables); node != eclist_end (tables); node = eclist_next (node))
      {
        AdblTable* table_info = adbl_dbtable (dbsession, eclist_data(node)); 
        if (isAssigned (table_info))
        {
          ecmap_append(self->tables, eclist_data(node), table_info);
          
          adbo_schema_addFKs (self, table_info);
        }        
      }
    }
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_schema_del (AdboSchema* pself)
{
  
}

//----------------------------------------------------------------------------------------

AdboObject adbo_schema_getFromTables (AdboSchema self, AdboContext context, AdboContainer parent, const EcString tablename, const EcString origin, AdboValue value)
{
  EcMapNode node;
  
  node = ecmap_find(self->tables, tablename);
  if (node == ecmap_end (self->tables))
  {
    return NULL;
  }
  
  eclogger_logformat (context->logger, LL_DEBUG, "ADBO", "{getdb} create '%s'", tablename, origin); 

  return adbo_object_new2 (parent, context, ADBO_OBJECT_NODE, ecmap_data(node), origin, value);
}

//----------------------------------------------------------------------------------------

AdboObject adbo_schema_get (AdboSchema self, AdboContext context, AdboContainer parent, const EcString tablename, const EcString origin, AdboValue value)
{
  EcMapNode node;
  
  node = ecmap_find(self->schemas, tablename);
  if (node == ecmap_end (self->schemas))
  {
    AdboObject obj = adbo_schema_getFromTables (self, context, parent, tablename, origin, value);
    if (isAssigned (obj))
    {
      ecmap_append(self->schemas, tablename, obj);
    }
    return obj;
  }
  else
  {
    // use cached object
    return ecmap_data (node);
  }
}

//----------------------------------------------------------------------------------------

void adbo_schema_ref (AdboSchema self, AdboContext context, AdboContainer parent, const EcString tablename, EcList objects, const EcString origin)
{
  EcList fks;
  EcListNode node1;
  EcMapNode node2 = ecmap_find(self->foreign_keys, tablename);
  
  if (node2 == ecmap_end(self->foreign_keys))
  {
    return; 
  }

  eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{fromdb} ref with origin '%s'", origin); 

  fks = ecmap_data(node2);
  for (node1 = eclist_first (fks); node1 != eclist_end (fks); node1 = eclist_next (node1))
  {
    AdboObject obj;

    AdblForeignKeyConstraint* fk = eclist_data(node1);
    
    if (ecstr_equal(origin, fk->name))
    {
      
    }
    else
    {
      eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{fromdb} got constraint '%s' to '%s'", fk->name, tablename); 
      
      obj = adbo_schema_get (self, context, parent, fk->name, origin, NULL);
      
      eclist_append(objects, obj);      
    }
  }

  
  
}

//----------------------------------------------------------------------------------------
