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
  
  EcList list;
  
  ubyte_t protect;
  
} EcUdcList;

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
    ecudc_destroy (&item);
  }
  
  ecmap_clear(self->map);
}

//----------------------------------------------------------------------------------------

void ecudc_node_destroy (void** pself)
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

int ecudc_node_del (EcUdcNode* self, const EcString name)
{
  EcUdc item;
  EcMapNode node = ecmap_find(self->map, name);
  
  if (node == ecmap_end(self->map))
  {
    return FALSE;
  }
  
  item = ecmap_data(node);
  
  ecmap_erase(node);
  
  ecudc_destroy(&item);
  
  return TRUE;  
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

EcUdc ecudc_node_ext (EcUdcNode* self, const EcString name)
{
  EcMapNode node = ecmap_find (self->map, name);
  EcUdc ret = NULL;
  
  if (node == ecmap_end(self->map))
  {
    return NULL;
  }
  
  ret = ecmap_data (node);
  
  ecmap_erase (node);
  
  return ret;
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

EcUdc ecudc_node_extract (EcUdcNode* self, void** cursor)
{
  EcMapNode node;
  EcUdc ret = NULL;
  
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
    node = ecmap_first (self->map);
  }
  
  if (node == ecmap_end (self->map))
  {
    return NULL;
  }
  
  ret = ecmap_data (node);
  
  *cursor = ecmap_erase (node);

  return ret;
}

//----------------------------------------------------------------------------------------

void ecudc_node_protect (EcUdcNode* self, ubyte_t mode)
{
  self->protect = mode;
}

//----------------------------------------------------------------------------------------

void* ecudc_list_new ()
{
  EcUdcList* self = ENTC_NEW (EcUdcList);

  self->list = eclist_new ();
  self->protect = FALSE;
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_list_clear (EcUdcList* self)
{
  EcListCursor cursor; eclist_cursor(self->list, &cursor);
  
  while (eclist_cnext (&cursor))
  {
    ecudc_destroy ((EcUdc*)&(cursor.value));    
  }
  
  eclist_clear(self->list);
}

//----------------------------------------------------------------------------------------

void ecudc_list_destroy (void** pself)
{
  EcUdcList* self = *pself;
  // if protected dont delete
  if (self->protect == FALSE)
  {
    ecudc_list_clear (self);
    
    eclist_delete (&(self->list));
    
    ENTC_DEL (pself, EcUdcList);    
  }
}

//----------------------------------------------------------------------------------------

void ecudc_list_add (EcUdcList* self, EcUdc* pnode)
{
  EcUdc node = *pnode;
  
  eclist_append(self->list, node);
  
  *pnode = NULL;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_list_next (EcUdcList* self, void** cursor)
{
  EcListNode node;
  
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  if (isAssigned (*cursor))
  {
    node = eclist_next(*cursor);
  }
  else
  {
    node = eclist_first(self->list);
  }
  
  if (node == eclist_end(self->list))
  {
    return NULL;
  }
  
  *cursor = node;
  return eclist_data(node);
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_list_extract (EcUdcList* self, void** cursor)
{
  EcListNode node;
  EcUdc ret = NULL;
  
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  if (isAssigned (*cursor))
  {
    node = eclist_next(*cursor);
  }
  else
  {
    node = eclist_first(self->list);
  }
  
  if (node == eclist_end(self->list))
  {
    return NULL;
  }
  
  ret = eclist_data (node);
  
  *cursor = eclist_erase (node);
  return ret;
}

//----------------------------------------------------------------------------------------

void ecudc_list_protect (EcUdcList* self, ubyte_t mode)
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

void ecudc_sitem_setS_o (EcUdcSItem* self, EcString* ptr)
{
  ecstr_replaceTO (&(self->value), *ptr);
  *ptr = NULL;
}

//----------------------------------------------------------------------------------------

const EcString ecudc_sitem_asString (EcUdcSItem* self)
{
  return self->value;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_create (uint_t type, const EcString name)
{
  EcUdc self = ENTC_NEW (struct EcUdc_s);
  
  self->type = type;
  self->name = ecstr_copy (name);
  
  switch (type) 
  {
    case ENTC_UDC_NODE: self->extension = ecudc_node_new (); 
      break;
    case ENTC_UDC_LIST: self->extension = ecudc_list_new (); 
      break;
    case ENTC_UDC_STRING: self->extension = ecudc_sitem_new (); 
      break;
    case ENTC_UDC_BYTE: self->extension = ENTC_NEW (ubyte_t);
      break;
    case ENTC_UDC_UINT32: self->extension = ENTC_NEW (uint32_t); 
      break;
    case ENTC_UDC_UINT64: self->extension = ENTC_NEW (uint64_t); 
      break;
    case ENTC_UDC_TIME: self->extension = ENTC_NEW (time_t); 
      break;
    case ENTC_UDC_CURSOR: self->extension = eccursor_create(); 
      break;
    case ENTC_UDC_FILEINFO: self->extension = ENTC_NEW (EcFileInfo_s); 
      break;
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_destroy (EcUdc* pself)
{
  EcUdc self = *pself;
  
  switch (self->type) 
  {
    case ENTC_UDC_NODE: ecudc_node_destroy (&(self->extension)); 
      break;
    case ENTC_UDC_LIST: ecudc_list_destroy (&(self->extension)); 
      break;
    case ENTC_UDC_STRING: ecudc_sitem_del (&(self->extension)); 
      break;
    case ENTC_UDC_REF: self->extension = NULL;
      break;
    case ENTC_UDC_BYTE: ENTC_DEL (&(self->extension), ubyte_t);
      break;
    case ENTC_UDC_UINT32: ENTC_DEL (&(self->extension), uint32_t);
      break;
    case ENTC_UDC_UINT64: ENTC_DEL (&(self->extension), uint64_t);
      break;
    case ENTC_UDC_TIME: ENTC_DEL (&(self->extension), time_t);
      break;
    case ENTC_UDC_CURSOR: eccursor_destroy ((EcCursor*)&(self->extension)); 
      break;
    case ENTC_UDC_FILEINFO: ENTC_DEL (&(self->extension), EcFileInfo_s);
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
    case ENTC_UDC_LIST: ecudc_list_add (self->extension, pnode); 
      break;
  }  
}

//----------------------------------------------------------------------------------------

int ecudc_del (EcUdc self, const EcString name)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: return ecudc_node_del (self->extension, name); 
  }  
  return FALSE;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node (EcUdc self, const EcString name)
{
  if (isAssigned (self))
  {
    switch (self->type) 
    {
      case ENTC_UDC_NODE: return ecudc_node_get (self->extension, name); 
    }        
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node_e (EcUdc self, const EcString name)
{
  if (isAssigned (self))
  {
    switch (self->type) 
    {
      case ENTC_UDC_NODE: return ecudc_node_ext (self->extension, name); 
    }        
  }
  return NULL;  
}

//----------------------------------------------------------------------------------------

const EcString ecudc_name (EcUdc self)
{
  return self->name;
}

//----------------------------------------------------------------------------------------

void ecudc_setName (EcUdc self, const EcString name)
{
  ecstr_replace(&(self->name), name);
}

//----------------------------------------------------------------------------------------

uint_t ecudc_type (EcUdc self)
{
  if (isAssigned (self))
  {
    return self->type;    
  }
  return 4000;
}

//----------------------------------------------------------------------------------------

void ecudc_setS (EcUdc self, const EcString value)
{
  switch (self->type) 
  {
    case ENTC_UDC_STRING: ecudc_sitem_setS (self->extension, value); break;
  }    
}

//----------------------------------------------------------------------------------------

void ecudc_setS_o (EcUdc self, EcString* ptr)
{
  switch (self->type) 
  {
    case ENTC_UDC_STRING: ecudc_sitem_setS_o (self->extension, ptr); break;
  }  
}

//----------------------------------------------------------------------------------------

EcFileInfo ecudc_asFileInfo (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_FILEINFO: return self->extension;
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

EcCursor ecudc_asCursor (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_CURSOR: return self->extension;
  }    
  return NULL;
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
    case ENTC_UDC_BYTE: memcpy(self->extension, &value, sizeof(ubyte_t)); 
    break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setUInt32 (EcUdc self, uint32_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT32: memcpy(self->extension, &value, sizeof(uint32_t)); 
      break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setUInt64 (EcUdc self, uint64_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT64: memcpy(self->extension, &value, sizeof(uint64_t)); 
    break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setTime (EcUdc self, const time_t* value)
{
  switch (self->type) 
  {
    case ENTC_UDC_TIME: memcpy(self->extension, value, sizeof(time_t)); 
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
    case ENTC_UDC_BYTE: return *((ubyte_t*)self->extension); 
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

uint32_t ecudc_asUInt32 (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT32: return *((uint32_t*)self->extension); 
    case ENTC_UDC_STRING:
    {
      const EcString h = ecudc_asString (self);
      if (isAssigned (h))
      {
        // can be transformed ?
        return atoi(h);
      }
    }
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

uint64_t ecudc_asUInt64 (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT64: return *((uint64_t*)self->extension); 
    case ENTC_UDC_STRING:
    {
      const EcString h = ecudc_asString (self);
      if (isAssigned (h))
      {
        // can be transformed ?
        return atoi(h);
      }
    }
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

const time_t* ecudc_asTime (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_TIME: return self->extension; 
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_next (EcUdc self, void** cursor)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: return ecudc_node_next (self->extension, cursor); 
    case ENTC_UDC_LIST: return ecudc_list_next (self->extension, cursor); 
  }        
  return NULL;  
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_extract (EcUdc self, void** cursor)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE: return ecudc_node_extract (self->extension, cursor); 
    case ENTC_UDC_LIST: return ecudc_list_extract (self->extension, cursor);       
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

void ecudc_protect (EcUdc self, ubyte_t mode)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: ecudc_node_protect (self->extension, mode); break;
    case ENTC_UDC_LIST: ecudc_list_protect (self->extension, mode); break;
  }        
}

//----------------------------------------------------------------------------------------

void* ecudc_get_asP (const EcUdc data, const EcString name, void* alt)
{
  const EcUdc res = ecudc_node (data, name);
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
  if (isAssigned (data))
  {
    const EcUdc res = ecudc_node (data, name);
    if (isAssigned (res))
    {
      return ecudc_asString(res);
    }
  }
  return alt; 
}

//----------------------------------------------------------------------------------------

ubyte_t ecudc_get_asB (const EcUdc self, const EcString name, ubyte_t alt)
{
  const EcUdc res = ecudc_node (self, name);
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

uint32_t ecudc_get_asUInt32 (const EcUdc self, const EcString name, uint32_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    uint32_t ret = ecudc_asUInt32 (res);
    if (ret == 0)
    {
      return alt;
    }
    else
    {
      return ret;
    }
  }
  else
  {
    return alt;
  }
}

//----------------------------------------------------------------------------------------

uint64_t ecudc_get_asUInt64 (const EcUdc self, const EcString name, uint64_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    uint64_t ret = ecudc_asUInt64 (res);
    if (ret == 0)
    {
      return alt;
    }
    else
    {
      return ret;
    }
  }
  else
  {
    return alt;
  }
}

//----------------------------------------------------------------------------------------

const time_t* ecudc_get_asTime (const EcUdc self, const EcString name, const time_t* alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    const time_t* ret = ecudc_asTime (res);
    if (ret == 0)
    {
      return alt;
    }
    else
    {
      return ret;
    }
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
  EcUdc item = ecudc_create (ENTC_UDC_REF, name);
  // set new value to item
  ecudc_setP(item, value);
  // add item to node 
  ecudc_add(node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asString (EcUdc node, const EcString name, const EcString value)
{
  // create new item as string
  EcUdc item = ecudc_create (ENTC_UDC_STRING, name);
  // set new value to item
  ecudc_setS(item, value);
  // add item to node 
  ecudc_add(node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asS_o (EcUdc node, const EcString name, EcString* ptr)
{
  // create new item as string
  EcUdc item = ecudc_create (ENTC_UDC_STRING, name);
  // set new value to item
  ecudc_setS_o(item, ptr);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asB (EcUdc node, const EcString name, ubyte_t value)
{
  EcUdc item = ecudc_create (ENTC_UDC_BYTE, name);
  // set new value to item
  ecudc_setB (item, value);
  // add item to node 
  ecudc_add (node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asUInt32 (EcUdc node, const EcString name, uint32_t value)
{
  EcUdc item = ecudc_create (ENTC_UDC_UINT32, name);
  // set new value to item
  ecudc_setUInt32 (item, value);
  // add item to node 
  ecudc_add (node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asUInt64 (EcUdc node, const EcString name, uint64_t value)
{
  EcUdc item = ecudc_create (ENTC_UDC_UINT64, name);
  // set new value to item
  ecudc_setUInt64 (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asTime (EcUdc node, const EcString name, const time_t* value)
{
  EcUdc item = ecudc_create (ENTC_UDC_TIME, name);
  // set new value to item
  ecudc_setTime (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_errcode (uint_t errcode)
{
  // create the default error code node
  EcUdc error = ecudc_create (ENTC_UDC_BYTE, "ErrorCode");
  // set the value
  ecudc_setB (error, errcode);
  // return
  return error;  
}

//----------------------------------------------------------------------------------------
