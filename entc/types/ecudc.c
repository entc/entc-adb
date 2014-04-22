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

#include "ecudc.h"

#include "ecmap.h"

//----------------------------------------------------------------------------------------

struct EcUdc_s
{
  
  uint_t type;
  
  void* extension;
  
  EcString name;
  
};

//----------------------------------------------------------------------------------------

typedef struct {
  
  EcMap map;
  
  ubyte_t protect;
  
} EcUdcNode;

typedef struct {
  
  EcString value;
  
} EcUdcSItem;

//----------------------------------------------------------------------------------------

void* ecudc_node_new ()
{
  EcUdcNode* self = ENTC_NEW (EcUdcNode);

  self->map = ecmap_new ();
  self->protect = FALSE;
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_node_clear (EcUdcNode* self)
{
  EcMapNode node;
  
  for (node = ecmap_first(self->map); node != ecmap_end(self->map); node = ecmap_next(node)) 
  {
    EcUdc item = ecmap_data(node);
    ecudc_del (&item);
  }
  
  ecmap_clear(self->map);
}

//----------------------------------------------------------------------------------------

void ecudc_node_del (void** pself)
{
  EcUdcNode* self = *pself;
  // if protected dont delete
  if (self->protect == FALSE)
  {
    ecudc_node_clear (self);
    
    ecmap_delete(&(self->map));
    
    ENTC_DEL (pself, EcUdcNode);    
  }
}

//----------------------------------------------------------------------------------------

void ecudc_node_add (EcUdcNode* self, EcUdc* pnode)
{
  EcUdc node = *pnode;
  
  if (ecstr_empty (node->name))
  {
    return;
  }
  
  ecmap_append(self->map, node->name, node);
  
  *pnode = NULL;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node_get (EcUdcNode* self, const EcString name)
{
  EcMapNode node = ecmap_find(self->map, name);
  
  if (node == ecmap_end(self->map))
  {
    return NULL;
  }
  
  return ecmap_data(node);
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node_next (EcUdcNode* self, void** cursor)
{
  EcMapNode node;
  
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  if (isAssigned (*cursor))
  {
    node = ecmap_next(*cursor);
  }
  else
  {
    node = ecmap_first(self->map);
  }
  
  if (node == ecmap_end(self->map))
  {
    return NULL;
  }
  
  *cursor = node;
  return ecmap_data(node);
}

//----------------------------------------------------------------------------------------

void ecudc_node_protect (EcUdcNode* self, ubyte_t mode)
{
  self->protect = mode;
}

//----------------------------------------------------------------------------------------

void* ecudc_sitem_new ()
{
  EcUdcSItem* self = ENTC_NEW (EcUdcSItem);
  
  self->value = ecstr_init ();
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_sitem_del (void** pself)
{
  EcUdcSItem* self = *pself;
  
  ecstr_delete(&(self->value));
  
  ENTC_DEL (pself, EcUdcSItem);
}

//----------------------------------------------------------------------------------------

void ecudc_sitem_setS (EcUdcSItem* self, const EcString value)
{
  ecstr_replace(&(self->value), value);
}

//----------------------------------------------------------------------------------------

const EcString ecudc_sitem_asString (EcUdcSItem* self)
{
  return self->value;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_new (uint_t type, const EcString name)
{
  EcUdc self = ENTC_NEW (struct EcUdc_s);
  
  self->type = type;
  self->name = ecstr_copy (name);
  
  switch (type) 
  {
    case ENTC_UDC_NODE: self->extension = ecudc_node_new (); 
      break;
    case ENTC_UDC_STRING: self->extension = ecudc_sitem_new (); 
      break;
    case ENTC_UDC_BYTE: (ubyte_t)self->extension = 0; 
      break;
    case ENTC_UDC_LONG: self->extension = ENTC_NEW (ulong_t); 
      break;
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_del (EcUdc* pself)
{
  EcUdc self = *pself;
  
  switch (self->type) 
  {
    case ENTC_UDC_NODE: ecudc_node_del (&(self->extension)); 
      break;
    case ENTC_UDC_STRING: ecudc_sitem_del (&(self->extension)); 
      break;
    case ENTC_UDC_REF: self->extension = NULL;
      break;
    case ENTC_UDC_BYTE: self->extension = NULL; 
      break;
    case ENTC_UDC_LONG: ENTC_DEL (&(self->extension), ulong_t);
      break;
  }
  // delete only if the content was deleted
  if (isNotAssigned (self->extension))
  {
    ecstr_delete(&(self->name));
    
    ENTC_DEL (pself, struct EcUdc_s);
  }
}

//----------------------------------------------------------------------------------------

void ecudc_add (EcUdc self, EcUdc* pnode)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: ecudc_node_add (self->extension, pnode); 
      break;
  }  
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_get (EcUdc self, const EcString name)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: return ecudc_node_get (self->extension, name); 
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

void ecudc_setS (EcUdc self, const EcString value)
{
  switch (self->type) 
  {
    case ENTC_UDC_STRING: ecudc_sitem_setS (self->extension, value); 
      break;
  }    
}

//----------------------------------------------------------------------------------------

void ecudc_setP (EcUdc self, void* ptr)
{
  switch (self->type) 
  {
    case ENTC_UDC_REF: self->extension = ptr; 
      break;
  }      
}

//----------------------------------------------------------------------------------------

void ecudc_setB (EcUdc self, ubyte_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_BYTE: (ubyte_t)self->extension = value; 
      break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setL (EcUdc self, ulong_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_LONG: *((ulong_t*)self->extension) = value; 
      break;
  }
}

//----------------------------------------------------------------------------------------

const EcString ecudc_asString (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_STRING: return ecudc_sitem_asString (self->extension); 
    default: return ecstr_init ();
  } 
}

//----------------------------------------------------------------------------------------

void* ecudc_asP (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_REF: return self->extension; 
  }        
  return NULL;
}

//----------------------------------------------------------------------------------------

ubyte_t ecudc_asB (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_BYTE: return (ubyte_t)self->extension; 
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

ulong_t ecudc_asL (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_LONG: return *((ulong_t*)self->extension); 
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_next (EcUdc self, void** cursor)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: return ecudc_node_next (self->extension, cursor); 
  }        
  return NULL;  
}

//----------------------------------------------------------------------------------------

void ecudc_protect (EcUdc self, ubyte_t mode)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: return ecudc_node_protect (self->extension, mode); 
  }        
}

//----------------------------------------------------------------------------------------

void* ecudc_get_asP (const EcUdc data, const EcString name, void* alt)
{
  const EcUdc res = ecudc_get (data, name);
  if (isAssigned (res))
  {
    return ecudc_asP(res);
  }
  else
  {
    return alt;
  }
}

//----------------------------------------------------------------------------------------

const EcString ecudc_get_asString (const EcUdc data, const EcString name, const EcString alt)
{
  const EcUdc res = ecudc_get (data, name);
  if (isAssigned (res))
  {
    return ecudc_asString(res);
  }
  else
  {
    return alt;
  }  
}

//----------------------------------------------------------------------------------------

ubyte_t ecudc_get_asB (const EcUdc self, const EcString name, ubyte_t alt)
{
  const EcUdc res = ecudc_get (self, name);
  if (isAssigned (res))
  {
    return ecudc_asB (res);
  }
  else
  {
    return alt;
  }  
}

//----------------------------------------------------------------------------------------

ulong_t ecudc_get_asL (const EcUdc self, const EcString name, ulong_t alt)
{
  const EcUdc res = ecudc_get (self, name);
  if (isAssigned (res))
  {
    return ecudc_asL (res);
  }
  else
  {
    return alt;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_add_asP (EcUdc node, const EcString name, void* value)
{
  // create new item as reference
  EcUdc item = ecudc_new (ENTC_UDC_REF, name);
  // set new value to item
  ecudc_setP(item, value);
  // add item to node 
  ecudc_add(node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asString (EcUdc node, const EcString name, const EcString value)
{
  // create new item as string
  EcUdc item = ecudc_new (ENTC_UDC_STRING, name);
  // set new value to item
  ecudc_setS(item, value);
  // add item to node 
  ecudc_add(node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asB (EcUdc node, const EcString name, ubyte_t value)
{
  // create new item as string
  EcUdc item = ecudc_new (ENTC_UDC_BYTE, name);
  // set new value to item
  ecudc_setB (item, value);
  // add item to node 
  ecudc_add (node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asL (EcUdc node, const EcString name, ulong_t value)
{
  // create new item as string
  EcUdc item = ecudc_new (ENTC_UDC_LONG, name);
  // set new value to item
  ecudc_setL (item, value);
  // add item to node 
  ecudc_add (node, &item);
}

//----------------------------------------------------------------------------------------

