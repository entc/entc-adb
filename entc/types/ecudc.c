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
  
} EcUdcNode;

typedef struct {
  
  EcString value;
  
} EcUdcSItem;

//----------------------------------------------------------------------------------------

void* ecudc_node_new ()
{
  EcUdcNode* self = ENTC_NEW (EcUdcNode);

  self->map = ecmap_new ();
  
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
  
  ecudc_node_clear (self);
  
  ecmap_delete(&(self->map));
  
  ENTC_DEL (pself, EcUdcNode);
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
    case 0: self->extension = ecudc_node_new (); 
      break;
    case 1: self->extension = ecudc_sitem_new (); 
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
    case 0: ecudc_node_del (&(self->extension)); 
      break;
    case 1: ecudc_sitem_del (&(self->extension)); 
      break;
  }
  
  ecstr_delete(&(self->name));
  
  ENTC_DEL (pself, struct EcUdc_s);
}

//----------------------------------------------------------------------------------------

void ecudc_add (EcUdc self, EcUdc* pnode)
{
  switch (self->type) 
  {
    case 0: ecudc_node_add (self->extension, pnode); 
      break;
  }  
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_get (EcUdc self, const EcString name)
{
  switch (self->type) 
  {
    case 0: return ecudc_node_get (self->extension, name); 
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

void ecudc_setS (EcUdc self, const EcString value)
{
  switch (self->type) 
  {
    case 1: ecudc_sitem_setS (self->extension, value); 
      break;
  }    
}

//----------------------------------------------------------------------------------------

const EcString ecudc_asString (EcUdc self)
{
  switch (self->type) 
  {
    case 1: return ecudc_sitem_asString (self->extension); 
    default: return ecstr_init ();
  } 
}

//----------------------------------------------------------------------------------------
