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

#include "adbo_node.h"
#include "adbo_substitute.h"
#include "adbo_item.h"

#include "adbo_container.h"

struct AdboObject_s
{

  uint_t type;

  EcString id;
  
  AdboValue value;

  void* extension;
  
  AdboContainer parent;

};

//----------------------------------------------------------------------------------------

AdboObject adbo_object_new (AdboContainer parent, uint_t type, EcXMLStream xmlstream, const EcString tag, EcLogger logger)
{
  AdboObject self = ENTC_NEW (struct AdboObject_s);

  self->value = NULL;
  
  self->type = type;
  self->id = ecstr_init ();
  
  if (isAssigned (xmlstream))
  {
    ecstr_replace (&(self->id), ecxmlstream_nodeAttribute (xmlstream, "id"));    
  }
  
  // create specific extension
  switch (type)
  {
    case ADBO_OBJECT_NODE:        self->extension = adbo_node_new (self, parent, xmlstream, logger); break;
    case ADBO_OBJECT_SUBSTITUTE:  self->extension = adbo_substitute_new(self, parent, xmlstream, logger); break;
    case ADBO_OBJECT_ITEM:        self->extension = adbo_item_new (self, parent, xmlstream, tag, logger); break;
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

void adbo_object_del (AdboObject* pself)
{
  ENTC_DEL (pself, struct AdboObject_s);
}

//----------------------------------------------------------------------------------------

AdboObject adbo_object_clone (const AdboObject oself, AdboContainer parent)
{
  AdboObject self = ENTC_NEW (struct AdboObject_s);
  
  self->type = oself->type;
  self->id = ecstr_copy (oself->id);
  self->value = adbo_value_clone (oself->value);
  
  // create specific extension
  switch (oself->type)
  {
    case ADBO_OBJECT_NODE:        self->extension = adbo_node_clone (oself->extension, parent); break;
    case ADBO_OBJECT_SUBSTITUTE:  self->extension = adbo_substitute_clone (oself->extension, parent); break;
    case ADBO_OBJECT_ITEM:        self->extension = adbo_item_clone (oself->extension); break;
      
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

int adbo_object_request (AdboObject self, AdboContext context)
{
  int ret = TRUE;
  // check
  if (isNotAssigned (self))
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{object} tried request on NULL object");
    return FALSE;
  }      
  // try to fill your value with data
  eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{object} request %i", self->type);

  // recursively continue with the type of this object
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        ret = adbo_node_request ((AdboNode)self->extension, context); break;
    case ADBO_OBJECT_SUBSTITUTE:  ret = adbo_substitute_request ((AdboSubstitute)self->extension, context); break;
  } 
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_object_update (AdboObject self, AdboContext context, int withTransaction)
{
  int ret = TRUE;
  // check
  if (isNotAssigned (self))
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{object} tried to update NULL object");
    return FALSE;
  }    
  // try to fill your value with data
  eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{object} update %i", self->type);

  // recursively continue with the type of this object
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        ret = adbo_node_update ((AdboNode)self->extension, context, withTransaction); break;
    case ADBO_OBJECT_SUBSTITUTE:  ret = adbo_substitute_update ((AdboSubstitute)self->extension, context, withTransaction); break;
  } 
  
  return ret;
}

//----------------------------------------------------------------------------------------

int adbo_object_delete (AdboObject self, AdboContext context, int withTransaction)
{
  int ret = TRUE;
  // check
  if (isNotAssigned (self))
  {
    eclogger_log (context->logger, LL_WARN, "ADBO", "{object} tried to delete NULL object");
    return FALSE;
  }  
  // try to fill your value with data
  eclogger_logformat (context->logger, LL_TRACE, "ADBO", "{object} delete %i", self->type);
  
  // recursively continue with the type of this object
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        ret = adbo_node_delete ((AdboNode)self->extension, context, withTransaction); break;
    case ADBO_OBJECT_SUBSTITUTE:  ret = adbo_substitute_delete ((AdboSubstitute)self->extension, context, withTransaction); break;
  } 
  
  return ret;  
}

//----------------------------------------------------------------------------------------

void adbo_objects_fromXml (AdboContainer container, EcXMLStream xmlstream, const EcString tag, EcLogger logger)
{
  if (isAssigned (xmlstream))
  {
    ENTC_XMLSTREAM_BEGIN
    
    if (ecxmlstream_isBegin (xmlstream, "node"))
    {
      eclogger_log(logger, LL_TRACE, "ADBO", "{parse} found node");

      adbo_container_add (container, adbo_object_new (container, ADBO_OBJECT_NODE, xmlstream, "node", logger));
    }
    else if (ecxmlstream_isBegin (xmlstream, "substitute"))
    {
      eclogger_log(logger, LL_TRACE, "ADBO", "{parse} found substitute");
      
      adbo_container_add (container, adbo_object_new (container, ADBO_OBJECT_SUBSTITUTE, xmlstream, "substitute", logger));
    }
    else if (ecxmlstream_isBegin (xmlstream, "item"))
    {
      eclogger_log(logger, LL_TRACE, "ADBO", "{parse} found item");
      
      adbo_container_add (container, adbo_object_new (container, ADBO_OBJECT_ITEM, xmlstream, "item", logger));
    }
    
    ENTC_XMLSTREAM_END (tag)
  }
}

//----------------------------------------------------------------------------------------

AdboValue adbo_getValue (AdboObject self)
{
  if (isAssigned (self))
  {
    return self->value;
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

void adbo_setValue (AdboObject self, AdboValue value)
{
  if (isNotAssigned (self))
  {
    return;
  }

  if (isAssigned (self->value))
  {

  }
  self->value = value;
}

//----------------------------------------------------------------------------------------

AdboObject adbo_at (AdboObject self, const EcString link)
{
  AdboObject obj = NULL;

  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        obj = adbo_node_at ((AdboNode)self->extension, link); break;
    case ADBO_OBJECT_SUBSTITUTE:  obj = adbo_substitute_at ((AdboSubstitute)self->extension, link); break;
  }    
  
  return obj;
}

//----------------------------------------------------------------------------------------

void adbo_strToStream (AdboObject self, EcStream stream)
{
  if (isNotAssigned (self))
  {
    return;
  }
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        adbo_node_strToStream ((AdboNode)self->extension, stream); break;
    case ADBO_OBJECT_SUBSTITUTE:  adbo_substitute_strToStream ((AdboSubstitute)self->extension, stream); break;
    case ADBO_OBJECT_ITEM:        adbo_item_strToStream (self, (AdboItem)self->extension, stream); break;
  } 
}

//----------------------------------------------------------------------------------------

EcString adbo_str (AdboObject self)
{
  if (isNotAssigned (self))
  {
    return NULL;
  }
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        return adbo_node_str ((AdboNode)self->extension);
    case ADBO_OBJECT_SUBSTITUTE:  return adbo_substitute_str ((AdboSubstitute)self->extension);
    case ADBO_OBJECT_ITEM:        return adbo_item_str (self);
  } 
  return NULL;
}

//----------------------------------------------------------------------------------------

int adbo_set (AdboObject self, const EcString value)
{
  if (isNotAssigned (self))
  {
    return FALSE;
  }

  if (isNotAssigned (self->value))
  {
    return FALSE;
  }

  adbo_value_set (self->value, value, ADBO_STATE_CHANGED);

  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_add (AdboObject self)
{
  return TRUE;
}

//----------------------------------------------------------------------------------------

int adbo_is (AdboObject self, const EcString link)
{
  if (isNotAssigned (self))
  {
    return FALSE;
  }
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        return adbo_node_is ((AdboNode)self->extension, link);
    case ADBO_OBJECT_SUBSTITUTE:  return adbo_substitute_is ((AdboSubstitute)self->extension, link);
    case ADBO_OBJECT_ITEM:        return adbo_item_is (self, link);
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------

void adbo_object_transaction (AdboObject self, int state)
{
  if (isAssigned (self->value))
  {
    if (state)
    {
      adbo_value_commit (self->value); 
    }
    else
    {
      adbo_value_rollback (self->value);     
    }    
  }
  
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        adbo_node_transaction ((AdboNode)self->extension, state); break;
    case ADBO_OBJECT_SUBSTITUTE:  adbo_substitute_transaction ((AdboSubstitute)self->extension, state); break;
    //case ADBO_OBJECT_ITEM: return adbo_item_transaction (self, link);
  }
}

//----------------------------------------------------------------------------------------

void adbo_dump_next (AdboObject self, int depth, int le, EcBuffer buffer, EcLogger logger)
{
  switch (self->type)
  {
    case ADBO_OBJECT_NODE:        adbo_node_dump (self, (AdboNode)self->extension, depth, le, buffer, logger); break;
    case ADBO_OBJECT_SUBSTITUTE:  adbo_substitute_dump (self, (AdboSubstitute)self->extension, depth, le, buffer, logger); break;
    case ADBO_OBJECT_ITEM:        adbo_item_dump (self, depth, le, buffer, logger); break;
  }  
}

//----------------------------------------------------------------------------------------

void adbo_dump (AdboObject self, EcLogger logger)
{
  EcBuffer buffer = ecstr_buffer (1024);
  
  eclogger_log(logger, LL_TRACE, "Q4EL", "============ adbo structure ============");  
  
  adbo_dump_next (self, 0, TRUE, buffer, logger);
    
  eclogger_log(logger, LL_TRACE, "Q4EL", "========================================");  
  
  ecstr_release (&buffer);
}

//----------------------------------------------------------------------------------------

const EcString adbo_dump_state (uint_t state)
{
  switch (state)
  {
    case ADBO_STATE_FIXED:     return "FIXED";
    case ADBO_STATE_ORIGINAL:  return "ORIGINAL";
    case ADBO_STATE_CHANGED:   return "CHANGED";
    case ADBO_STATE_DELETED:   return "DELETED";
    case ADBO_STATE_NONE:      return "NONE";
    case ADBO_STATE_INSERTED:  return "INSERTED";
  }
  return "UNKNOWN";
}

//----------------------------------------------------------------------------------------


