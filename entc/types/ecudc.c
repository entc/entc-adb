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
  
  EcList list;
  
} EcUdcList;

typedef struct {
  
  EcString value;
  
} EcUdcSItem;

//----------------------------------------------------------------------------------------

void* ecudc_node_new (EcAlloc alloc)
{
  EcUdcNode* self = ECMM_NEW (EcUdcNode);

  self->map = ecmap_create (alloc);
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_node_clear (EcAlloc alloc, EcUdcNode* self)
{
  EcMapNode node;
  
  for (node = ecmap_first(self->map); node != ecmap_end(self->map); node = ecmap_next(node)) 
  {
    EcUdc item = ecmap_data(node);
    ecudc_destroy (alloc, &item);
  }
  
  ecmap_clear (alloc, self->map);
}

//----------------------------------------------------------------------------------------

void ecudc_node_destroy (EcAlloc alloc, void** pself)
{
  EcUdcNode* self = *pself;
  // if protected dont delete
  ecudc_node_clear (alloc, self);
  
  ecmap_destroy (alloc, &(self->map));
  
  ECMM_DEL (pself, EcUdcNode);    
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

int ecudc_node_del (EcAlloc alloc, EcUdcNode* self, const EcString name)
{
  EcUdc item;
  EcMapNode node = ecmap_find(self->map, name);
  
  if (node == ecmap_end(self->map))
  {
    return FALSE;
  }
  
  item = ecmap_data(node);
  
  ecmap_erase (self->map, node);
  
  ecudc_destroy(alloc, &item);
  
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
  
  ecmap_erase (self->map, node);
  
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

EcUdc ecudc_map_e (EcUdcNode* self, void** cursor)
{
  EcMapNode node;
  EcUdc ret = NULL;
  
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  node = *cursor;
  
  if (node == ecmap_end (self->map))
  {
    return NULL;
  }
  
  ret = ecmap_data (node);
  
  *cursor = ecmap_erase (self->map, node);

  return ret;
}

//----------------------------------------------------------------------------------------

void* ecudc_list_new (EcAlloc alloc)
{
  EcUdcList* self = ECMM_NEW (EcUdcList);

  self->list = eclist_create_ex (alloc);
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_list_clear (EcAlloc alloc, EcUdcList* self)
{
  EcListCursor cursor; eclist_cursor(self->list, &cursor);
  
  while (eclist_cnext (&cursor))
  {
    ecudc_destroy (alloc, (EcUdc*)&(cursor.value));    
  }
  
  eclist_clear(self->list);
}

//----------------------------------------------------------------------------------------

void ecudc_list_destroy (EcAlloc alloc, void** pself)
{
  EcUdcList* self = *pself;
  // if protected dont delete
  ecudc_list_clear (alloc, self);
  
  eclist_free_ex (EC_ALLOC, &(self->list));
  
  ECMM_DEL (pself, EcUdcList);    
}

//----------------------------------------------------------------------------------------

void ecudc_list_add (EcUdcList* self, EcUdc* pnode)
{
  EcUdc node = *pnode;
  
  eclist_append_ex (EC_ALLOC, self->list, node);
  
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

EcUdc ecudc_list_e (EcUdcList* self, void** cursor)
{
  EcListNode node;
  EcUdc ret = NULL;
  
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  node = *cursor;
  
  if (node == eclist_end(self->list))
  {
    return NULL;
  }
  
  ret = eclist_data (node);
  
  *cursor = eclist_erase (EC_ALLOC, self->list, node);
  return ret;
}

//----------------------------------------------------------------------------------------

void* ecudc_sitem_new (EcAlloc alloc)
{
  EcUdcSItem* self = ECMM_NEW (EcUdcSItem);
  
  self->value = ecstr_init ();
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_sitem_del (EcAlloc alloc, void** pself)
{
  EcUdcSItem* self = *pself;
  
  ecstr_delete(&(self->value));
  
  ECMM_DEL (pself, EcUdcSItem);
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

EcUdc ecudc_create (EcAlloc alloc, uint_t type, const EcString name)
{
  EcUdc self = ECMM_NEW (struct EcUdc_s);
  
  self->type = type;
  self->name = ecstr_copy (name);
  
  switch (type) 
  {
    case ENTC_UDC_NODE: self->extension = ecudc_node_new (alloc); 
      break;
    case ENTC_UDC_LIST: self->extension = ecudc_list_new (alloc); 
      break;
    case ENTC_UDC_STRING: self->extension = ecudc_sitem_new (alloc); 
      break;
    case ENTC_UDC_BYTE: self->extension = ECMM_NEW (byte_t);
      break;
    case ENTC_UDC_UBYTE: self->extension = ECMM_NEW (ubyte_t);
      break;
    case ENTC_UDC_INT16: self->extension = ECMM_NEW (int16_t); 
      break;
    case ENTC_UDC_UINT16: self->extension = ECMM_NEW (uint16_t); 
      break;
    case ENTC_UDC_INT32: self->extension = ECMM_NEW (int32_t); 
      break;
    case ENTC_UDC_UINT32: self->extension = ECMM_NEW (uint32_t); 
      break;
    case ENTC_UDC_INT64: self->extension = ECMM_NEW (int64_t); 
      break;
    case ENTC_UDC_UINT64: self->extension = ECMM_NEW (uint64_t); 
      break;
    case ENTC_UDC_FLOAT: self->extension = ECMM_NEW (float);
      break;
    case ENTC_UDC_DOUBLE: self->extension = ECMM_NEW (double); 
      break;
    case ENTC_UDC_TIME: self->extension = ECMM_NEW (time_t);
      break;
    case ENTC_UDC_CURSOR: self->extension = eccursor_create(); 
      break;
    case ENTC_UDC_FILEINFO:
    {
      EcFileInfo h = ECMM_NEW (EcFileInfo_s);
      
      h->name = ecstr_init ();
      
      self->extension = h;
    }
    break;
    case ENTC_UDC_TABLEINFO:
    {
      EcTableInfo h = ECMM_NEW (EcTableInfo_s); 
      
      h->name = ecstr_init ();
      
      self->extension = h;
    }
    break;
    case ENTC_UDC_SET:
    {
      EcSet h = ECMM_NEW (EcSet_s);
      
      h->setid = NULL;
      h->content = NULL;
      
      self->extension = h;
    }
    break;
    case ENTC_UDC_USERINFO:
    {
      EcUserInfo h = ECMM_NEW (EcUserInfo_s);
      
      h->name = ecstr_init ();
      
      self->extension = h; 
    }
    break;
    case ENTC_UDC_ERROR:
    {
      EcError h = ECMM_NEW (EcError_s);
      
      h->text = ecstr_init ();
      h->code = 0;
      
      self->extension = h;
    }
    break;
    case ENTC_UDC_METHOD:
    {
      EcMethod h = ECMM_NEW (EcMethod_s);
      
      h->name = ecstr_init ();
      h->version = 0;
      h->error = NULL;
      h->params = NULL;
      h->result = NULL;
      
      self->extension = h;
    }
    break;
  }
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_destroy (EcAlloc alloc, EcUdc* pself)
{
  EcUdc self = *pself;
  
  switch (self->type) 
  {
    case ENTC_UDC_NODE: ecudc_node_destroy (alloc, &(self->extension)); 
      break;
    case ENTC_UDC_LIST: ecudc_list_destroy (alloc, &(self->extension)); 
      break;
    case ENTC_UDC_STRING: ecudc_sitem_del (alloc, &(self->extension)); 
      break;
    case ENTC_UDC_REF: self->extension = NULL;
      break;
    case ENTC_UDC_BYTE: ENTC_DEL (&(self->extension), byte_t);
      break;
    case ENTC_UDC_UBYTE: ENTC_DEL (&(self->extension), ubyte_t);
      break;
    case ENTC_UDC_INT16: ENTC_DEL (&(self->extension), int16_t);
      break;
    case ENTC_UDC_UINT16: ENTC_DEL (&(self->extension), uint16_t);
      break;
    case ENTC_UDC_INT32: ENTC_DEL (&(self->extension), int32_t);
      break;
    case ENTC_UDC_UINT32: ENTC_DEL (&(self->extension), uint32_t);
      break;
    case ENTC_UDC_INT64: ENTC_DEL (&(self->extension), int64_t);
      break;
    case ENTC_UDC_UINT64: ENTC_DEL (&(self->extension), uint64_t);
      break;
    case ENTC_UDC_FLOAT: ENTC_DEL (&(self->extension), float);
      break;
    case ENTC_UDC_DOUBLE: ENTC_DEL (&(self->extension), double);
      break;
    case ENTC_UDC_TIME: ENTC_DEL (&(self->extension), time_t);
      break;
    case ENTC_UDC_CURSOR: eccursor_destroy ((EcCursor*)&(self->extension)); 
      break;
    case ENTC_UDC_FILEINFO:
    {
      EcFileInfo h = self->extension;
      
      ecstr_delete (&(h->name));
      
      ENTC_DEL (&h, EcFileInfo_s);
      self->extension = NULL;
    }
      break;
    case ENTC_UDC_TABLEINFO: 
    {
      EcTableInfo h = self->extension;
      
      ecstr_delete (&(h->name));
      
      ENTC_DEL (&h, EcTableInfo_s);
      self->extension = NULL;
    }
    break;
    case ENTC_UDC_SET:
    {
      EcSet h = self->extension;
      
      if (isAssigned (h->setid))
      {
        ecudc_destroy (alloc, &(h->setid));
      }

      if (isAssigned (h->content))
      {
        ecudc_destroy (alloc, &(h->content));
      }
                     
      ENTC_DEL (&h, EcSet_s);
      self->extension = NULL;
    }
    break;
    case ENTC_UDC_USERINFO: 
    {
      EcUserInfo h = self->extension;
      
      ecstr_delete (&(h->name));
      
      ENTC_DEL (&h, EcUserInfo_s);
      self->extension = NULL;
    }
    break; 
    case ENTC_UDC_ERROR:
    {
      EcError h = self->extension;

      ecstr_delete (&(h->text));
      
      ENTC_DEL (&h, EcError_s);
      self->extension = NULL;
    }
    break;
    case ENTC_UDC_METHOD:
    {
      EcMethod h = self->extension;

      ecstr_delete (&(h->name));

      if (isAssigned (h->error))
      {
        ecudc_destroy (alloc, &(h->error));
      }
      
      if (isAssigned (h->params))
      {
        ecudc_destroy (alloc, &(h->params));
      }

      if (isAssigned (h->result))
      {
        ecudc_destroy (alloc, &(h->result));
      }
      
      ENTC_DEL (&h, EcMethod_s);
      self->extension = NULL;
    }
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

int ecudc_del (EcAlloc alloc, EcUdc self, const EcString name)
{
  switch (self->type) 
  {
    case ENTC_UDC_NODE: return ecudc_node_del (alloc, self->extension, name); 
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

EcSet ecudc_asSet (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_SET: return self->extension;
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

EcUserInfo ecudc_asUserInfo (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_USERINFO: return self->extension;
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

EcTableInfo ecudc_asTableInfo (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_TABLEINFO: return self->extension;
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

EcError ecudc_asError (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_ERROR: return self->extension;
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

EcMethod ecudc_asMethod (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_METHOD: return self->extension;
  }    
  return NULL;
}

//----------------------------------------------------------------------------------------

void ecudc_setP (EcUdc self, void* ptr)
{
  switch (self->type) 
  {
    case ENTC_UDC_REF: self->extension = ptr; break;
  }      
}

//----------------------------------------------------------------------------------------

void ecudc_setByte (EcUdc self, byte_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_BYTE: memcpy(self->extension, &value, sizeof(byte_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setUByte (EcUdc self, ubyte_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_UBYTE: memcpy(self->extension, &value, sizeof(ubyte_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setInt16 (EcUdc self, int16_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_INT16: memcpy(self->extension, &value, sizeof(int16_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setUInt16 (EcUdc self, uint16_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT16: memcpy(self->extension, &value, sizeof(uint16_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setInt32 (EcUdc self, int32_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_INT32: memcpy(self->extension, &value, sizeof(int32_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setUInt32 (EcUdc self, uint32_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT32: memcpy(self->extension, &value, sizeof(uint32_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setInt64 (EcUdc self, int64_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_INT64: memcpy(self->extension, &value, sizeof(int64_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setUInt64 (EcUdc self, uint64_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT64: memcpy(self->extension, &value, sizeof(uint64_t)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setFloat (EcUdc self, float value)
{
  switch (self->type) 
  {
    case ENTC_UDC_FLOAT: memcpy(self->extension, &value, sizeof(float)); break;
  }
}

//----------------------------------------------------------------------------------------

void ecudc_setDouble (EcUdc self, double value)
{
  switch (self->type) 
  {
    case ENTC_UDC_DOUBLE: memcpy(self->extension, &value, sizeof(double)); break;
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

EcString ecudc_getString (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_STRING:  return ecstr_copy (ecudc_sitem_asString (self->extension)); 
    case ENTC_UDC_BYTE:    return ecstr_long (ecudc_asByte (self));
    case ENTC_UDC_UBYTE:   return ecstr_long (ecudc_asUByte (self));
    case ENTC_UDC_INT16:   return ecstr_long (ecudc_asInt16 (self));
    case ENTC_UDC_UINT16:  return ecstr_long (ecudc_asUInt16 (self));
    case ENTC_UDC_INT32:   return ecstr_long (ecudc_asInt32 (self));
    case ENTC_UDC_UINT32:  return ecstr_long (ecudc_asUInt32 (self));
    case ENTC_UDC_INT64:   return ecstr_long (ecudc_asInt64 (self));
    case ENTC_UDC_UINT64:  return ecstr_long (ecudc_asUInt64 (self));
  } 
  return ecstr_init ();
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

byte_t ecudc_asByte (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_BYTE: return *((byte_t*)self->extension);
  }
  return 0;
}

//----------------------------------------------------------------------------------------

ubyte_t ecudc_asUByte (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_UBYTE: return *((ubyte_t*)self->extension); 
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

int16_t ecudc_asInt16 (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_INT16: return *((int16_t*)self->extension); 
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

uint16_t ecudc_asUInt16 (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_UINT16: return *((uint16_t*)self->extension); 
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

int32_t ecudc_asInt32 (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_INT32: return *((int32_t*)self->extension); 
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

int64_t ecudc_asInt64 (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_INT64: return *((int64_t*)self->extension); 
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

float ecudc_asFloat (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_FLOAT: return *((float*)self->extension); 
    case ENTC_UDC_STRING:
    {
      const EcString h = ecudc_asString (self);
      if (isAssigned (h))
      {
        // can be transformed ?
#pragma warning( push )
#pragma warning( disable : 4244)
        return atof(h);
#pragma warning( pop )
      }
    }
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

double ecudc_asDouble (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_DOUBLE: return *((double*)self->extension); 
    case ENTC_UDC_STRING:
    {
      const EcString h = ecudc_asString (self);
      if (isAssigned (h))
      {
        // can be transformed ?
        return atof(h);
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

EcUdc ecudc_cursor_e (EcUdc self, void** cursor)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE: return ecudc_map_e (self->extension, cursor); 
    case ENTC_UDC_LIST: return ecudc_list_e (self->extension, cursor);       
  }
  return NULL;
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

byte_t ecudc_get_asByte (const EcUdc self, const EcString name, byte_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    return ecudc_asByte (res);
  }
  else
  {
    return alt;
  }  
}

//----------------------------------------------------------------------------------------

ubyte_t ecudc_get_asUByte (const EcUdc self, const EcString name, ubyte_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    return ecudc_asUByte (res);
  }
  else
  {
    return alt;
  }  
}

//----------------------------------------------------------------------------------------

int16_t ecudc_get_asInt16 (const EcUdc self, const EcString name, int16_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    int16_t ret = ecudc_asInt16 (res);
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

uint16_t ecudc_get_asUInt16 (const EcUdc self, const EcString name, uint16_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    uint16_t ret = ecudc_asUInt16 (res);
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

int32_t ecudc_get_asInt32 (const EcUdc self, const EcString name, int32_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    int32_t ret = ecudc_asInt32 (res);
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

int64_t ecudc_get_asInt64 (const EcUdc self, const EcString name, int64_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    int64_t ret = ecudc_asInt64 (res);
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

float ecudc_get_asFloat (const EcUdc self, const EcString name, float alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    float ret = ecudc_asFloat (res);
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

double ecudc_get_asDouble (const EcUdc self, const EcString name, double alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    double ret = ecudc_asDouble (res);
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

void ecudc_add_asP (EcAlloc alloc, EcUdc node, const EcString name, void* value)
{
  // create new item as reference
  EcUdc item = ecudc_create (alloc, ENTC_UDC_REF, name);
  // set new value to item
  ecudc_setP(item, value);
  // add item to node 
  ecudc_add(node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asString (EcAlloc alloc, EcUdc node, const EcString name, const EcString value)
{
  // create new item as string
  EcUdc item = ecudc_create (alloc, ENTC_UDC_STRING, name);
  // set new value to item
  ecudc_setS(item, value);
  // add item to node 
  ecudc_add(node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asS_o (EcAlloc alloc, EcUdc node, const EcString name, EcString* ptr)
{
  // create new item as string
  EcUdc item = ecudc_create (alloc, ENTC_UDC_STRING, name);
  // set new value to item
  ecudc_setS_o(item, ptr);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asByte (EcAlloc alloc, EcUdc node, const EcString name, byte_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_BYTE, name);
  // set new value to item
  ecudc_setByte (item, value);
  // add item to node 
  ecudc_add (node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asUByte (EcAlloc alloc, EcUdc node, const EcString name, ubyte_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_UBYTE, name);
  // set new value to item
  ecudc_setUByte (item, value);
  // add item to node 
  ecudc_add (node, &item);
}

//----------------------------------------------------------------------------------------

void ecudc_add_asInt16 (EcAlloc alloc, EcUdc node, const EcString name, int16_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_INT16, name);
  // set new value to item
  ecudc_setInt16 (item, value);
  // add item to node 
  ecudc_add (node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asUInt16 (EcAlloc alloc, EcUdc node, const EcString name, uint16_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_UINT16, name);
  // set new value to item
  ecudc_setUInt16 (item, value);
  // add item to node 
  ecudc_add (node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asInt32 (EcAlloc alloc, EcUdc node, const EcString name, int32_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_INT32, name);
  // set new value to item
  ecudc_setInt32 (item, value);
  // add item to node 
  ecudc_add (node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asUInt32 (EcAlloc alloc, EcUdc node, const EcString name, uint32_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_UINT32, name);
  // set new value to item
  ecudc_setUInt32 (item, value);
  // add item to node 
  ecudc_add (node, &item);  
}

//----------------------------------------------------------------------------------------

void ecudc_add_asInt64 (EcAlloc alloc, EcUdc node, const EcString name, int64_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_INT64, name);
  // set new value to item
  ecudc_setInt64 (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asUInt64 (EcAlloc alloc, EcUdc node, const EcString name, uint64_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_UINT64, name);
  // set new value to item
  ecudc_setUInt64 (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asFloat (EcAlloc alloc, EcUdc node, const EcString name, float value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_FLOAT, name);
  // set new value to item
  ecudc_setFloat (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asDouble (EcAlloc alloc, EcUdc node, const EcString name, double value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_DOUBLE, name);
  // set new value to item
  ecudc_setDouble (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

void ecudc_add_asTime (EcAlloc alloc, EcUdc node, const EcString name, const time_t* value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_TIME, name);
  // set new value to item
  ecudc_setTime (item, value);
  // add item to node 
  ecudc_add (node, &item);    
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_errcode (EcAlloc alloc, uint_t errcode)
{
  // create the default error code node
  EcUdc error = ecudc_create (alloc, ENTC_UDC_BYTE, "ErrorCode");
  // set the value
  ecudc_setByte (error, errcode);
  // return
  return error;  
}

//----------------------------------------------------------------------------------------
