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

static void __STDCALL ecudc_node_item_destroy (void* key, void* val)
{
  EcUdc item = val;
  EcString h = key;
  
  ecudc_destroy (EC_ALLOC, &item);
  
  ecstr_delete (&h);
}

//----------------------------------------------------------------------------------------

void* ecudc_node_new (EcAlloc alloc)
{
  EcUdcNode* self = ECMM_NEW (EcUdcNode);

  self->map = ecmap_create (NULL, ecudc_node_item_destroy);
  
  return self;
}

//----------------------------------------------------------------------------------------

void* __STDCALL ecudc_node_onCloneKey (void* ptr)
{
  return ecstr_copy (ptr);
}

//----------------------------------------------------------------------------------------

void* __STDCALL ecudc_node_onCloneVal (void* ptr)
{
  return ecudc_clone (EC_ALLOC, ptr);
}

//----------------------------------------------------------------------------------------

void* ecudc_node_clone (EcAlloc alloc, EcUdcNode* orig)
{
  EcUdcNode* self = ECMM_NEW (EcUdcNode);
  
  self->map = ecmap_clone (orig->map, ecudc_node_onCloneKey, ecudc_node_onCloneVal);
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_node_clear (EcAlloc alloc, EcUdcNode* self)
{
  ecmap_clear (self->map);
}

//----------------------------------------------------------------------------------------

void ecudc_node_destroy (EcAlloc alloc, void** pself)
{
  EcUdcNode* self = *pself;
  // if protected dont delete
  ecudc_node_clear (alloc, self);
  
  ecmap_destroy (&(self->map));
  
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
  
  // use string for key type
  ecmap_insert (self->map, ecstr_copy(node->name), node);
  
  *pnode = NULL;
}

//----------------------------------------------------------------------------------------

int ecudc_node_del (EcAlloc alloc, EcUdcNode* self, const EcString name)
{
  EcMapNode node = ecmap_find (self->map, (void*)name);
  
  if (node == NULL)
  {
    return FALSE;
  }
  
  ecmap_erase (self->map, node);
  
  return TRUE;  
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node_get (EcUdcNode* self, const EcString name)
{
  EcMapNode node = ecmap_find (self->map, (void*)name);
  
  if (node == NULL)
  {
    return NULL;
  }
  
  return ecmap_node_value (node);
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node_ext (EcUdcNode* self, const EcString name)
{
  EcMapNode node = ecmap_find (self->map, (void*)name);
  EcUdc ret = NULL;
  
  if (node == NULL)
  {
    return NULL;
  }
  
  {
    EcMapNode h = ecmap_extract (self->map, node);
    
    ret = ecmap_node_value (h);
    
    {
      EcString j = ecmap_node_key (h);
      
      ecstr_delete(&j);
    }

    ecmap_node_destroy (&h);
  }
  
  return ret;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_node_next (EcUdcNode* self, void** pcursor)
{
  if (pcursor == NULL)
  {
    return NULL;
  }
  
  if (*pcursor == NULL)
  {
    // initialize
    *pcursor = ecmap_cursor_create (self->map, LIST_DIR_NEXT);
  }
  
  {
    EcMapCursor* cursor = *pcursor;
    
    if (ecmap_cursor_next (cursor))
    {
      return ecmap_node_value (cursor->node);
    }
    
    // done
    ecmap_cursor_destroy (&cursor);
    
    *pcursor = NULL;
    
    return NULL;
  }
}

//----------------------------------------------------------------------------------------

uint32_t ecudc_node_size (EcUdcNode* self)
{
  return ecmap_size (self->map);
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_map_e (EcUdcNode* self, void** pcursor)
{
  if (pcursor == NULL)
  {
    return NULL;
  }
  
  if (*pcursor == NULL)
  {
    // initialize
    *pcursor = ecmap_cursor_create (self->map, LIST_DIR_NEXT);
  }
  
  {
    EcMapCursor* cursor = *pcursor;
    
    if (ecmap_cursor_next (cursor))
    {
      EcString key;
	  EcUdc val;

      EcMapNode node = ecmap_cursor_extract (self->map, cursor);
      
      // delete key
      key = ecmap_node_key (node);
      ecstr_delete(&key);
	  
      val = ecmap_node_value (node);
      ecmap_node_destroy (&node);
	  
      return val;
    }
    
    // done
    ecmap_cursor_destroy (&cursor);
    
    *pcursor = NULL;
    
    return NULL;
  }
}

//----------------------------------------------------------------------------------------

static int __STDCALL ecudc_list_onDestroy (void* ptr)
{
  EcUdc h = ptr;
  
  ecudc_destroy (EC_ALLOC, &h);
  
  return 0;
}

//----------------------------------------------------------------------------------------

void* ecudc_list_new (EcAlloc alloc)
{
  EcUdcList* self = ECMM_NEW (EcUdcList);

  self->list = eclist_create (ecudc_list_onDestroy);
  
  return self;
}

//----------------------------------------------------------------------------------------

void* __STDCALL ecudc_list_clone_onClone (void* ptr)
{
  return ecudc_clone (EC_ALLOC, ptr);
}

//----------------------------------------------------------------------------------------

void* ecudc_list_clone (EcAlloc alloc, EcUdcList* orig)
{
  EcUdcList* self = ECMM_NEW (EcUdcList);
  
  self->list = eclist_clone (orig->list, ecudc_list_clone_onClone);
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecudc_list_clear (EcAlloc alloc, EcUdcList* self)
{
  eclist_clear (self->list);
}

//----------------------------------------------------------------------------------------

void ecudc_list_destroy (EcAlloc alloc, void** pself)
{
  EcUdcList* self = *pself;
  // if protected dont delete
  ecudc_list_clear (alloc, self);
  
  eclist_destroy (&(self->list));
  
  ECMM_DEL (pself, EcUdcList);    
}

//----------------------------------------------------------------------------------------

void ecudc_list_add (EcUdcList* self, EcUdc* pnode)
{
  EcUdc node = *pnode;
  
  eclist_push_back (self->list, node);
  
  *pnode = NULL;
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_list_next (EcUdcList* self, void** cursor)
{
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  if (*cursor == NULL)
  {
    *cursor = eclist_cursor_create (self->list, LIST_DIR_NEXT);
  }
  
  if (eclist_cursor_next (*cursor))
  {
    return eclist_data(((EcListCursor*)(*cursor))->node);
  }
  else
  {
    eclist_cursor_destroy ((EcListCursor**)cursor);
    
    return NULL;
  }
}

//----------------------------------------------------------------------------------------

EcUdc ecudc_list_e (EcUdcList* self, void** cursor)
{
  if (isNotAssigned (cursor))
  {
    return NULL;
  }
  
  if (*cursor == NULL)
  {
    *cursor = eclist_cursor_create (self->list, LIST_DIR_NEXT);
  }
  
  if (eclist_cursor_next (*cursor))
  {
    return eclist_cursor_extract (self->list, *cursor);
  }
  else
  {
    eclist_cursor_destroy ((EcListCursor**)cursor);
    
    return NULL;
  }
}

//----------------------------------------------------------------------------------------

uint32_t ecudc_list_size (EcUdcList* self)
{
  return eclist_size (self->list);
}

//----------------------------------------------------------------------------------------

void* ecudc_sitem_clone (EcAlloc alloc, EcUdcSItem* orig)
{
  EcUdcSItem* self = ECMM_NEW (EcUdcSItem);
  
  self->value = ecstr_copy(orig->value);
  
  return self;
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
    case ENTC_UDC_NUMBER: self->extension = ECMM_NEW (int64_t);
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
      h->path = ecstr_init ();
      
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
      
      h->acc_type = 0;
      h->name = ecstr_init ();
      h->extras = NULL;
      
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
    case ENTC_UDC_BUFFER:
    {
      self->extension = NULL;  // initialize with NULL
    }
    break;
    case ENTC_UDC_BOOL:
    {
      self->extension = NULL; // false
    }
    break;
    case ENTC_UDC_NONE:
    {
      self->extension = NULL; // just nothing
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
    case ENTC_UDC_BOOL: self->extension = NULL;
      break;
    case ENTC_UDC_NUMBER: ENTC_DEL (&(self->extension), int64_t);
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
      ecstr_delete (&(h->path));
      
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
      
      if (isAssigned (h->extras))
      {
        ecudc_destroy (alloc, &(h->extras));
      }
      
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
    case ENTC_UDC_BUFFER: 
    {
      if (isAssigned (self->extension))
      {
        ecbuf_destroy ((void*)&(self->extension));
      }
        
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

EcUdc ecudc_clone (EcAlloc alloc, const EcUdc orig)
{
  EcUdc self = ECMM_NEW (struct EcUdc_s);
  
  self->type = orig->type;
  self->name = ecstr_copy (orig->name);

  switch (orig->type)
  {
    case ENTC_UDC_NODE:
    {
      self->extension = ecudc_node_clone (alloc, orig->extension);
      break;
    }
    case ENTC_UDC_LIST:
    {
      self->extension = ecudc_list_clone (alloc, orig->extension);
      break;
    }
    case ENTC_UDC_STRING:
    {
      self->extension = ecudc_sitem_clone (alloc, orig->extension);
      break;
    }
    case ENTC_UDC_REF:
    {
      self->extension = orig->extension;
      break;
    }
    case ENTC_UDC_BOOL:
    {
      self->extension = orig->extension;
      break;
    }
    case ENTC_UDC_NUMBER:
    {
      self->extension = ECMM_NEW (int64_t); memcpy(self->extension, orig->extension, sizeof(int64_t));
      break;
    }
    case ENTC_UDC_DOUBLE:
    {
      self->extension = ECMM_NEW (double); memcpy(self->extension, orig->extension, sizeof(double));
      break;
    }
    case ENTC_UDC_TIME:
    {
      self->extension = ECMM_NEW (time_t); memcpy(self->extension, orig->extension, sizeof(time_t));
      break;
    }
    case ENTC_UDC_CURSOR:
    {
      
      break;
    }
    case ENTC_UDC_FILEINFO:
    {
      EcFileInfo h = orig->extension;
      
      break;
    }
    case ENTC_UDC_TABLEINFO:
    {
      EcTableInfo h = orig->extension;
      
      break;
    }
    case ENTC_UDC_SET:
    {
      EcSet h = orig->extension;
      
      break;
    }
    case ENTC_UDC_USERINFO:
    {
      EcUserInfo h = orig->extension;
      
      break;
    }
    case ENTC_UDC_ERROR:
    {
      EcError h = orig->extension;
      
      break;
    }
    case ENTC_UDC_METHOD:
    {
      EcMethod h = orig->extension;
      
      break;
    }
    case ENTC_UDC_BUFFER:
    {
      break;
    }
  }
  
  return self;
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

void ecudc_merge (EcUdc* dest, EcUdc* part)
{
  if (*part)
  {
    if (*dest == NULL)
    {
      *dest = *part;
      *part = NULL;
    }
    else
    {
      // merge params
      void* cursor = NULL;
      EcUdc item;
      
      for (item = ecudc_cursor_e (*part, &cursor); item; item = ecudc_cursor_e (*part, &cursor))
      {
        // try to avoid conflicts
        EcUdc hnode = ecudc_node (*dest, ecudc_name(item));
        if (hnode == NULL)
        {
          ecudc_add (*dest, &item);
        }
        else
        {
          ecudc_destroy(EC_ALLOC, &item);
        }
      }
      
      ecudc_destroy(EC_ALLOC, part);
    }
  }
}

//----------------------------------------------------------------------------------------

uint32_t ecudc_size (EcUdc self)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE: return ecudc_node_size (self->extension);
    case ENTC_UDC_LIST: return ecudc_list_size (self->extension);
  }
  
  return 0;
}

//----------------------------------------------------------------------------------------

void ecudc_refNumber (EcUdc self, int64_t** ref)
{
  switch (self->type)
  {
    case ENTC_UDC_NUMBER:
    {
      *ref = self->extension;
      break;
    }
  }
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

void ecudc_setB_o (EcUdc self, EcBuffer* ptr)
{
  switch (self->type) 
  {
    case ENTC_UDC_BUFFER: 
    {
      if (isAssigned (self->extension))
      {
        ecbuf_destroy ((void*)&(self->extension));
      }
      
      self->extension = *ptr;
      *ptr = NULL;
    }
    break;
  }  
}

//----------------------------------------------------------------------------------------

void ecudc_setBool (EcUdc self, int val)
{
  switch (self->type) 
  {
    case ENTC_UDC_BOOL: 
    {
      self->extension = (val == TRUE) ? (void*)1 : NULL;
    }
    break;
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

void ecudc_setNumber (EcUdc self, int64_t value)
{
  switch (self->type) 
  {
    case ENTC_UDC_NUMBER: memcpy(self->extension, &value, sizeof(int64_t)); break;
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
    case ENTC_UDC_TIME: memcpy(self->extension, value, sizeof(time_t)); break;
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
    case ENTC_UDC_STRING: return ecstr_copy (ecudc_sitem_asString (self->extension));
    case ENTC_UDC_NUMBER: return ecstr_long (ecudc_asNumber (self));
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

int64_t ecudc_asNumber (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_NUMBER: return *((int64_t*)self->extension);
    case ENTC_UDC_STRING:
    {
      const EcString h = ecudc_asString (self);
      if (isAssigned (h))
      {
#ifdef _WIN32
		return atol (h);
#else
        return atoll(h);
#endif
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
    case ENTC_UDC_NUMBER: return *((int64_t*)self->extension);
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

EcBuffer ecudc_asB (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_BUFFER: return self->extension; 
  }        
  return 0;
}

//----------------------------------------------------------------------------------------

int ecudc_asBool (EcUdc self)
{
  switch (self->type) 
  {
    case ENTC_UDC_BOOL: return self->extension != NULL; 
  }     
  
  return FALSE;
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

int64_t ecudc_get_asNumber (const EcUdc self, const EcString name, int64_t alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    return ecudc_asNumber (res);
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
    return ecudc_asDouble (res);
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

EcBuffer ecudc_get_asB (const EcUdc self, const EcString name, const EcBuffer alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    EcBuffer ret = ecudc_asB (res);
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

int ecudc_get_asBool (const EcUdc self, const EcString name, int alt)
{
  const EcUdc res = ecudc_node (self, name);
  if (isAssigned (res))
  {
    return ecudc_asBool (res);
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

void ecudc_add_asB_o (EcAlloc alloc, EcUdc node, const EcString name, EcBuffer* ptr)
{
  // create new item as string
  EcUdc item = ecudc_create (alloc, ENTC_UDC_BUFFER, name);
  // set new value to item
  ecudc_setB_o (item, ptr);
  // add item to node 
  ecudc_add (node, &item);      
}

//----------------------------------------------------------------------------------------

void ecudc_add_asNumber (EcAlloc alloc, EcUdc node, const EcString name, int64_t value)
{
  EcUdc item = ecudc_create (alloc, ENTC_UDC_NUMBER, name);
  // set new value to item
  ecudc_setNumber (item, value);
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
  EcUdc error = ecudc_create (alloc, ENTC_UDC_NUMBER, "ErrorCode");
  // set the value
  ecudc_setNumber (error, errcode);
  // return
  return error;  
}

//----------------------------------------------------------------------------------------
