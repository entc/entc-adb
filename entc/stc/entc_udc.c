#include "entc_udc.h"

// cape includes
#include "sys/entc_types.h"
#include "stc/entc_list.h"
#include "stc/entc_map.h"

//-----------------------------------------------------------------------------

struct EntcUdc_s
{
  u_t type;
  
  void* data;
  
  EntcString name;
};

//----------------------------------------------------------------------------------------

static void __STDCALL entc_udc_node_onDel (void* key, void* val)
{
  EntcUdc h = val; entc_udc_del (&h);
}

//----------------------------------------------------------------------------------------

static void __STDCALL entc_udc_list_onDel (void* ptr)
{
  EntcUdc h = ptr; entc_udc_del (&h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_new (u_t type, const EntcString name)
{
  EntcUdc self = ENTC_NEW(struct EntcUdc_s);
  
  self->type = type;
  self->data = NULL;
  
  self->name = entc_str_cp (name);
  
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      self->data = entc_map_new (NULL, entc_udc_node_onDel);
      break;
    }
    case ENTC_UDC_LIST:
    {
      self->data = entc_list_new (entc_udc_list_onDel);
      break;
    }
    case ENTC_UDC_STRING:
    {
      self->data = NULL;
      break;
    }
    case ENTC_UDC_FLOAT:
    {
      self->data = ENTC_NEW(double);
      break;
    }
  }
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_udc_del (EntcUdc* p_self)
{
  EntcUdc self = *p_self;
  
  if (self == NULL)
  {
    return;
  }
  
  entc_str_del (&(self->name));
  
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      entc_map_del ((EntcMap*)&(self->data));
      break;
    }
    case ENTC_UDC_LIST:
    {
      entc_list_del ((EntcList*)&(self->data));
      break;
    }
    case ENTC_UDC_STRING:
    {
      entc_str_del ((EntcString*)&(self->data));
      break;
    }
    case ENTC_UDC_FLOAT:
    {
      ENTC_DEL(&(self->data), double);
      break;
    }
  }
  
  ENTC_DEL(p_self, struct EntcUdc_s);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_cp (const EntcUdc self)
{
  
}

//-----------------------------------------------------------------------------

const EntcString  entc_udc_name (const EntcUdc self)
{
  return self->name;
}

//-----------------------------------------------------------------------------

u_t entc_udc_type (const EntcUdc self)
{
  return self->type;
}

//-----------------------------------------------------------------------------

void* entc_udc_data (const EntcUdc self)
{
  switch (self->type)
  {
    case ENTC_UDC_STRING:
    {
      return self->data;
    }
    case ENTC_UDC_FLOAT:
    {
      return self->data;
    }
    case ENTC_UDC_NUMBER:
    {
      return &(self->data);
    }
    case ENTC_UDC_BOOL:
    {
      return &(self->data);
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------

number_t entc_udc_size (const EntcUdc self)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      return entc_map_size (self->data);
    }
    case ENTC_UDC_LIST:
    {
      return entc_list_size (self->data);
    }
    default:
    {
      return 0;
    }
  }
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add (EntcUdc self, EntcUdc* p_item)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      EntcUdc h = *p_item; 
      
      entc_map_insert (self->data, h->name, h);
      
      *p_item = NULL;
      
      return h;
    }
    case ENTC_UDC_LIST:
    {
      EntcUdc h = *p_item; 
      
      entc_list_push_back (self->data, h);
      
      *p_item = NULL;
      
      return h;
    }
    default:
    {
      // we can't add this item, but we can delete it
      entc_udc_del (p_item);
      
      return NULL;
    }
  }
}

//-----------------------------------------------------------------------------

void entc_udc_set_name (const EntcUdc self, const EntcString name)
{
  entc_str_replace_cp (&(self->name), name);  
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_name (EntcUdc self, EntcUdc* p_item, const EntcString name)
{
  entc_udc_set_name (*p_item, name);
  
  return entc_udc_add (self, p_item);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_get (EntcUdc self, const EntcString name)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      EntcMapNode n = entc_map_find (self->data, (void*)name);
      
      if (n)
      {
        return entc_map_node_value (n);
      }
      else
      {
        return NULL;
      }
    }
    default:
    {
      return NULL;
    }
  }
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_ext (EntcUdc self, const EntcString name)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      EntcMapNode n = entc_map_find (self->data, (void*)name);
      
      if (n)
      {
        n = entc_map_extract (self->data, n);  
        
        EntcUdc h = entc_map_node_value (n);
        
        entc_map_node_del (&n);
        
        return h;
      }
      else
      {
        return NULL;
      }
    }
    default:
    {
      return NULL;
    }
  }  
}

//-----------------------------------------------------------------------------

void entc_udc_set_s_cp (EntcUdc self, const EntcString val)
{
  switch (self->type)
  {
    case ENTC_UDC_STRING:
    {
      entc_str_replace_cp ((EntcString*)&(self->data), val);
    }
  }   
}

//-----------------------------------------------------------------------------

void entc_udc_set_s_mv (EntcUdc self, EntcString* p_val)
{
  switch (self->type)
  {
    case ENTC_UDC_STRING:
    {
      entc_str_replace_mv ((EntcString*)&(self->data), p_val);
    }
  }   
}

//-----------------------------------------------------------------------------

void entc_udc_set_n (EntcUdc self, number_t val)
{
  switch (self->type)
  {
    case ENTC_UDC_NUMBER:
    {
      self->data = (void*)val;
    }
  }  
}

//-----------------------------------------------------------------------------

void entc_udc_set_f (EntcUdc self, double val)
{
  switch (self->type)
  {
    case ENTC_UDC_FLOAT:
    {
      double* h = self->data;
      
      *h = val;
    }
  }  
}

//-----------------------------------------------------------------------------

void entc_udc_set_b (EntcUdc self, int val)
{
  switch (self->type)
  {
    case ENTC_UDC_BOOL:
    {
      self->data = val ? (void*)1 : NULL;
    }
  }  
}

//-----------------------------------------------------------------------------

const EntcString entc_udc_s (EntcUdc self, const EntcString alt)
{
  switch (self->type)
  {
    case ENTC_UDC_STRING:
    {
      return self->data;
    }
    default:
    {
      return alt;
    }
  }    
}

//-----------------------------------------------------------------------------

EntcString entc_udc_s_mv (EntcUdc self, const EntcString alt)
{
  switch (self->type)
  {
    case ENTC_UDC_STRING:
    {
      EntcString h = self->data;
      
      self->data = NULL;
      
      return h;
    }
    default:
    {
      return entc_str_cp (alt);
    }
  }  
}

//-----------------------------------------------------------------------------

number_t entc_udc_n (EntcUdc self, number_t alt)
{
  switch (self->type)
  {
    case ENTC_UDC_NUMBER:
    {
      return (number_t)(self->data);
    }
    default:
    {
      return alt;
    }
  }  
}

//-----------------------------------------------------------------------------

double entc_udc_f (EntcUdc self, double alt)
{
  switch (self->type)
  {
    case ENTC_UDC_FLOAT:
    {
      double* h = self->data;
      
      return *h;
    }
    default:
    {
      return alt;
    }
  }  
}

//-----------------------------------------------------------------------------

int entc_udc_b (EntcUdc self, int alt)
{
  switch (self->type)
  {
    case ENTC_UDC_BOOL:
    {
      return self->data ? TRUE : FALSE;
    }
    default:
    {
      return alt;
    }
  }  
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_s_cp (EntcUdc self, const EntcString name, const EntcString val)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_STRING, name);
  
  entc_udc_set_s_cp (h, val);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_s_mv (EntcUdc self, const EntcString name, EntcString* p_val)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_STRING, name);
  
  entc_udc_set_s_mv (h, p_val);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_n (EntcUdc self, const EntcString name, number_t val)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_NUMBER, name);
  
  entc_udc_set_n (h, val);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_f (EntcUdc self, const EntcString name, double val)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_FLOAT, name);
  
  entc_udc_set_f (h, val);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_b (EntcUdc self, const EntcString name, int val)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_BOOL, name);
  
  entc_udc_set_b (h, val);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_node (EntcUdc self, const EntcString name)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_NODE, name);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_add_list (EntcUdc self, const EntcString name)
{
  EntcUdc h = entc_udc_new (ENTC_UDC_LIST, name);
  
  return entc_udc_add (self, &h);
}

//-----------------------------------------------------------------------------

const EntcString entc_udc_get_s (EntcUdc self, const EntcString name, const EntcString alt)
{
  EntcUdc h = entc_udc_get (self, name);
  
  if (h)
  {
    return entc_udc_s (h, alt);
  }
  
  return alt;
}

//-----------------------------------------------------------------------------

number_t entc_udc_get_n (EntcUdc self, const EntcString name, number_t alt)
{
  EntcUdc h = entc_udc_get (self, name);
  
  if (h)
  {
    return entc_udc_n (h, alt);
  }
  
  return alt;
}

//-----------------------------------------------------------------------------

double entc_udc_get_f (EntcUdc self, const EntcString name, double alt)
{
  EntcUdc h = entc_udc_get (self, name);
  
  if (h)
  {
    return entc_udc_f (h, alt);
  }
  
  return alt; 
}

//-----------------------------------------------------------------------------

int entc_udc_get_b (EntcUdc self, const EntcString name, int alt)
{
  EntcUdc h = entc_udc_get (self, name);
  
  if (h)
  {
    return entc_udc_b (h, alt);
  }
  
  return alt; 
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_get_node (EntcUdc self, const EntcString name)
{
  EntcUdc h = entc_udc_get (self, name);
  
  if (h)
  {
    if (h->type == ENTC_UDC_NODE)
    {
      return h;
    }
  }
  
  return NULL;  
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_get_list (EntcUdc self, const EntcString name)
{
  EntcUdc h = entc_udc_get (self, name);
  
  if (h)
  {
    if (h->type == ENTC_UDC_LIST)
    {
      return h;
    }
  }
  
  return NULL;   
}

//-----------------------------------------------------------------------------

EntcString entc_udc_ext_s (EntcUdc self, const EntcString name)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      
    }
    default:
    {
      return NULL;
    }
  }  
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_ext_node (EntcUdc self, const EntcString name)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      EntcUdc h = entc_udc_get (self, name);
      
      if (h)
      {
        if (h->type == ENTC_UDC_NODE)
        {
          return entc_udc_ext (self, name);
        }
      }
    }
    default:
    {
      return NULL;
    }
  }  
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_ext_list (EntcUdc self, const EntcString name)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      EntcUdc h = entc_udc_get (self, name);
      
      if (h)
      {
        if (h->type == ENTC_UDC_LIST)
        {
          return entc_udc_ext (self, name);
        }
      }
    }
    default:
    {
      return NULL;
    }
  }  
}

//-----------------------------------------------------------------------------

EntcUdcCursor* entc_udc_cursor_new (EntcUdc self, int direction)
{
  EntcUdcCursor* cursor = ENTC_NEW(EntcUdcCursor);
  
  cursor->direction = direction;
  cursor->position = -1;
  cursor->item = NULL;
  
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      cursor->data = entc_map_cursor_create (self->data, direction);
      cursor->type = ENTC_UDC_NODE;
      
      break;
    }
    case ENTC_UDC_LIST:
    {
      cursor->data = entc_list_cursor_create (self->data, direction);
      cursor->type = ENTC_UDC_LIST;
      
      break;
    }
    default:
    {
      cursor->data = NULL;
      cursor->type = 0;
      
      break;
    }
  } 
  
  return cursor;
}

//-----------------------------------------------------------------------------

void entc_udc_cursor_del (EntcUdcCursor** p_cursor)
{
  EntcUdcCursor* cursor = *p_cursor;
  
  switch (cursor->type)
  {
    case ENTC_UDC_NODE:
    {
      entc_map_cursor_destroy ((EntcMapCursor**)&(cursor->data));
      break;
    }
    case ENTC_UDC_LIST:
    {
      entc_list_cursor_destroy ((EntcListCursor**)&(cursor->data));
      break;
    }
  } 
  
  ENTC_DEL(p_cursor, EntcUdcCursor);
}

//-----------------------------------------------------------------------------

int entc_udc_cursor_next (EntcUdcCursor* cursor)
{
  if (cursor->data)
  {
    switch (cursor->type)
    {
      case ENTC_UDC_NODE:
      {
        EntcMapCursor* c = cursor->data;
        
        int res = entc_map_cursor_next (c);
        
        if (res)
        {
          cursor->position++;
          cursor->item = entc_map_node_value (c->node);
        }
        
        return res;
      }
      case ENTC_UDC_LIST:
      {
        EntcListCursor* c = cursor->data;
        
        int res = entc_list_cursor_next (c);
        
        if (res)
        {
          cursor->position++;
          cursor->item = entc_list_node_data (c->node);
        }
        
        return res;
      }
    }
  }
  
  return FALSE;
}

//-----------------------------------------------------------------------------

int entc_udc_cursor_prev (EntcUdcCursor* cursor)
{
  if (cursor->data)
  {
    switch (cursor->type)
    {
      case ENTC_UDC_NODE:
      {
        EntcMapCursor* c = cursor->data;
        
        int res = entc_map_cursor_prev (c);
        
        if (res)
        {
          cursor->position--;
          cursor->item = entc_map_node_value (c->node);
        }
        
        return res;
      }
      case ENTC_UDC_LIST:
      {
        EntcListCursor* c = cursor->data;
        
        int res = entc_list_cursor_prev (c);
        
        if (res)
        {
          cursor->position--;
          cursor->item = entc_list_node_data (c->node);
        }
        
        return res;
      }
    }
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------

void entc_udc_cursor_rm (EntcUdc self, EntcUdcCursor* cursor)
{
  
}

//-----------------------------------------------------------------------------

EntcUdc entc_udc_cursor_ext (EntcUdc self, EntcUdcCursor* cursor)
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      EntcMapNode n = entc_map_cursor_extract (self->data, cursor->data);
      
      EntcUdc h = entc_map_node_value (n);
      
      entc_map_node_del (&n);
      
      return h;
    }
    case ENTC_UDC_LIST:
    {
      return entc_list_cursor_extract (self->data, cursor->data);
      break;
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------

void entc_udc_print (const EntcUdc self) 
{
  switch (self->type)
  {
    case ENTC_UDC_NODE:
    {
      
      break;
    }
    case ENTC_UDC_LIST:
    {

      
      break;
    }
    case ENTC_UDC_STRING:
    {
      printf ("UDC [string] : %s\n", (char*)self->data);
      
      break;
    }
    case ENTC_UDC_NUMBER:
    {
      printf ("UDC [number] : %ld\n", (number_t)(self->data));
            
      break;
    }
  }
}

//-----------------------------------------------------------------------------
