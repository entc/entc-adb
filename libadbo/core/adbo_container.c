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

#include "adbo_container.h"

#include "adbo_object.h"

struct AdboContainer_s
{
  
  uint_t type;
  
  void* ext;
  
  AdboContainer parent;
  
};

//----------------------------------------------------------------------------------------

AdboContainer adbo_container_new (uint_t type, AdboContainer parent)
{
  AdboContainer self = ENTC_NEW (struct AdboContainer_s);
  
  self->type = type;
  self->parent = parent;

  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      self->ext = eclist_new ();
    }
    break;
  }
        
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_container_del (AdboContainer* pself)
{  
  AdboContainer self = *pself;
  
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        AdboObject obj = (AdboObject)eclist_data (node);
        adbo_object_del (&obj);
      }  

      eclist_delete (&objects);
    }
    break;
  }
  
  ENTC_DEL (pself, struct AdboContainer_s);
}

//----------------------------------------------------------------------------------------

AdboContainer adbo_container_clone (const AdboContainer oself, AdboContainer parent, container_callback_fct fct, void* ptr1, void* ptr2)
{
  AdboContainer self = ENTC_NEW (struct AdboContainer_s);
  
  self->type = oself->type;
  self->parent = parent;
  
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList oobjects = (EcList)oself->ext;
      self->ext = eclist_new ();

      for (node = eclist_first (oobjects); node != eclist_end (oobjects); node = eclist_next (node))
      {
        AdboObject oobj = (AdboObject)eclist_data (node);
        AdboObject nobj = adbo_object_clone (oobj, self);
        
        eclist_append (self->ext, nobj);
        
        fct (ptr1, ptr2, adbo_getValue (oobj), adbo_getValue (nobj));
      }  
    }
    break;   
  }

  return self;  
}

//----------------------------------------------------------------------------------------

void adbo_container_add (AdboContainer self, AdboObject obj)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      eclist_append ((EcList)self->ext, obj);
    }
    break;
  }
}

//----------------------------------------------------------------------------------------

void adbo_container_query (AdboContainer self, AdblQuery* query)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        // get the value from each object
        AdboValue value = adbo_getValue ((AdboObject)eclist_data (node));
        if (isAssigned (value))
        {
          if (adbo_value_getState (value) != ADBO_STATE_FIXED)
          {
            // add the dbcolumn
            const EcString dbcolumn = adbo_value_getDBColumn (value);
            if (ecstr_valid (dbcolumn))
            {
              adbl_query_addColumn (query, dbcolumn, 0);
            }        
          }
        }
      }      
    }
    break;
  }
}

//----------------------------------------------------------------------------------------

void adbo_container_attrs (AdboContainer self, AdblAttributes* attrs)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;

      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        // get the value from each object
        AdboValue value = adbo_getValue ((AdboObject)eclist_data (node));
        if (isAssigned (value))
        {
          if (adbo_value_getState (value) == ADBO_STATE_CHANGED)
          {
            // add the dbcolumn
            const EcString dbcolumn = adbo_value_getDBColumn (value);
            if (ecstr_valid (dbcolumn))
            {
              adbl_attrs_addChar (attrs, dbcolumn, adbo_value_cget (value, self));
            }
          }
        }
      }  
    }
    break;
  }  
}

//----------------------------------------------------------------------------------------

void adbo_container_set (AdboContainer self, AdblCursor* cursor, EcLogger logger)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        // get the value from each object
        AdboValue value = adbo_getValue ((AdboObject)eclist_data (node));
        if (isAssigned (value))
        {
          if (adbo_value_getState (value) != ADBO_STATE_FIXED)
          {
            // if we have added this colum now set the value
            const EcString dbcolumn = adbo_value_getDBColumn (value);
            if (ecstr_valid (dbcolumn))
            {
              const EcString data = adbl_dbcursor_nextdata (cursor);
              
              eclogger_logformat (logger, LL_TRACE, "ADBO", "{request} set values '%s' -> '%s'", dbcolumn, data);
              // override value
              adbo_value_set (value, data, ADBO_STATE_ORIGINAL);
            }      
          }
        }
      }      
    }
    break;
  }  
}

//----------------------------------------------------------------------------------------

int adbo_container_request (AdboContainer self, AdboContext context)
{
  int ret = TRUE;

  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      // iterate through all objects and again trigger the request
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        adbo_object_request ((AdboObject)eclist_data (node), context);
      }      
    }
    break;
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_container_update (AdboContainer self, AdboContext context)
{
  int ret = TRUE;
  
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      // iterate through all objects and again trigger the request
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        ret = ret && adbo_object_update ((AdboObject)eclist_data (node), context, FALSE);
      }      
    }
    break;
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_container_delete (AdboContainer self, AdboContext context)
{
  int ret = TRUE;
  
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      // iterate through all objects and again trigger the request
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        ret = ret && adbo_object_delete ((AdboObject)eclist_data (node), context, FALSE);
      }      
    }
    break;
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

void adbo_container_transaction (AdboContainer self, int state)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      // iterate through all objects and again trigger the request
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        adbo_object_transaction ((AdboObject)eclist_data (node), state);
      }      
    }
    break;
  }  
}

//----------------------------------------------------------------------------------------

void adbo_container_str (AdboContainer self, EcStream stream)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      ecstream_appendc (stream, '{');
      
      // iterate through all objects and first update them
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        if (node != eclist_first (objects))
        {
          ecstream_appendc (stream, ',');
        }
        adbo_strToStream ((AdboObject)eclist_data (node), stream);
      }
      
      ecstream_appendc (stream, '}');
    }
    break;
  }
}

//----------------------------------------------------------------------------------------

AdboObject adbo_container_get (AdboContainer self, const EcString item)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        AdboObject obj = (AdboObject)eclist_data (node);
        
        if (adbo_is (obj, item))
        {
          return obj;
        }
      }
    }
    break;
  }  
  return NULL;
}

//----------------------------------------------------------------------------------------

AdboObject adbo_container_at (AdboContainer self, const EcString link)
{
  AdboObject obj = NULL;
  EcString part1;
  EcString part2;
  
  if (ecstr_empty (link))
  {
    return NULL;
  }
  
  part1 = ecstr_init ();
  part2 = ecstr_init ();

  // follow link
  if (ecstr_split (link, &part1, &part2, '/'))
  {
    if ( ecstr_equal (part1, ".."))
    {
      // go one level up
      obj = adbo_container_at (self->parent, part2);
    }
    else
    {
      obj = adbo_container_get (self, part1);
      if (isAssigned (obj))
      {
        obj = adbo_at (obj, part2);
      }
    }
  }
  else
  {
    obj = adbo_container_get (self, link);
  }
  
  ecstr_delete (&part1);
  ecstr_delete (&part2);

  return obj;
}

//----------------------------------------------------------------------------------------

void adbo_container_iterate (AdboContainer self, iterator_callback_fct fct, void* ptr)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        fct (ptr, eclist_data (node));
      }
    }
    break;
  }
}

//----------------------------------------------------------------------------------------

AdboContainer adbo_container_parent (AdboContainer self)
{
  return self->parent;
}

//----------------------------------------------------------------------------------------

void adbo_container_dump (AdboContainer self, int tab, EcBuffer buffer, EcLogger logger)
{
  switch (self->type)
  {
    case ADBO_CONTAINER_NODE:
    {
      EcListNode node;
      EcList objects = (EcList)self->ext;
      
      for (node = eclist_first (objects); node != eclist_end (objects); node = eclist_next (node))
      {
        adbo_dump_next (eclist_data (node), tab, eclist_next (node) == eclist_end (objects), buffer, logger);
      }
    }
      break;
  }  
}

//----------------------------------------------------------------------------------------
