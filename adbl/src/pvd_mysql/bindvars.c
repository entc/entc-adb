#include "bindvars.h"

//-----------------------------------------------------------------------------

struct AdblBindVars_s
{  
  MYSQL_BIND* binds;
  
  int size;
  
  int pos;
  
};

//------------------------------------------------------------------------------------------------------

AdblBindVars adbl_bindvars_new (int size)
{
  number_t bindSize;
  
  AdblBindVars self = CAPE_NEW (struct AdblBindVars_s);
  
  self->size = size;
  self->pos = 0;
  
  bindSize = sizeof(MYSQL_BIND) * size;
  if (bindSize > 0)
  {
    // create a new array of binds
    self->binds = CAPE_ALLOC(bindSize);    
    memset (self->binds, 0, bindSize);
  }
  else
  {
    self->binds = NULL;
  }
  
  return self;
}

//------------------------------------------------------------------------------------------------------

void adbl_bindvars_del (AdblBindVars* p_self)
{
  AdblBindVars self = *p_self;
  
  {
    int i;
    
    for (i = 0; i < self->size; i++)
    {
      MYSQL_BIND* bind = &(self->binds[i]);
      
      if (bind->buffer && bind->pack_length)
      {
        CAPE_FREE(bind->buffer);
      }
    }
  }
  
  // release the array of the bind variables
  CAPE_FREE (self->binds);
  
  CAPE_DEL (p_self, struct AdblBindVars_s);
}

//------------------------------------------------------------------------------------------------------

MYSQL_BIND* adbl_bindvars_binds (AdblBindVars self)
{
  return self->binds;
}

//------------------------------------------------------------------------------------------------------

void adbl_bind_set (MYSQL_BIND* bind, CapeUdc item)
{
  switch (cape_udc_type (item))
  {
    case CAPE_UDC_STRING:
    {
      const CapeString h = cape_udc_s (item, NULL);
      
      bind->buffer_type = MYSQL_TYPE_STRING;
      bind->buffer = (char*)h;
      bind->buffer_length = strlen (h);
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0;
      
      break;
    }
    case CAPE_UDC_NUMBER:
    {
      bind->buffer_type = MYSQL_TYPE_LONG;
      bind->buffer = cape_udc_data (item);
      bind->buffer_length = 0;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0; 
      bind->is_unsigned = 0;
      
      break;
    }
    case CAPE_UDC_FLOAT:
    {
      bind->buffer_type = MYSQL_TYPE_DOUBLE;
      bind->buffer = cape_udc_data (item);
      bind->buffer_length = 0;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0; 
      bind->is_unsigned = 0;
      
      break;
    }
    case CAPE_UDC_BOOL:
    {
      bind->buffer_type = MYSQL_TYPE_LONG;    // use long because original is int
      bind->buffer = cape_udc_data (item);
      bind->buffer_length = 0;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0; 
      bind->is_unsigned = 0;
      
      break;
    }
  }
}

//------------------------------------------------------------------------------------------------------

void adbl_bind_add (MYSQL_BIND* bind, CapeUdc item)
{
  switch (cape_udc_type (item))
  {
    case CAPE_UDC_STRING:
    {
      bind->buffer_type = MYSQL_TYPE_STRING;
      
      bind->buffer = CAPE_ALLOC(1024);
      memset (bind->buffer, 0, 1024);
      
      bind->buffer_length = 1024;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0;
      
      bind->pack_length = 1;
      
      break;
    }
    case CAPE_UDC_NUMBER:
    {
      bind->buffer_type = MYSQL_TYPE_LONG;
      
      bind->buffer = CAPE_ALLOC(8);
      memset (bind->buffer, 0, 8);
      
      bind->buffer_length = 0;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0; 
      bind->is_unsigned = 0;
      
      bind->pack_length = 1;
      
      break;
    }
    case CAPE_UDC_FLOAT:
    {
      bind->buffer_type = MYSQL_TYPE_DOUBLE;
      bind->buffer = CAPE_ALLOC(8);
      bind->buffer_length = 0;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0; 
      bind->is_unsigned = 0;
      
      bind->pack_length = 1;
      
      break;
    }
    case CAPE_UDC_BOOL:
    {
      bind->buffer_type = MYSQL_TYPE_LONG;    // use long because original is int
      bind->buffer = CAPE_ALLOC(8);
      bind->buffer_length = 0;
      bind->is_null = 0;
      bind->length = 0;
      bind->error = 0; 
      bind->is_unsigned = 0;
      
      bind->pack_length = 1;
      
      break;
    }
  }  
}

//------------------------------------------------------------------------------------------------------

void adbl_bind_get (MYSQL_BIND* bind, CapeUdc item)
{
  switch (cape_udc_type (item))
  {
    case CAPE_UDC_STRING:
    {
      // copy the buffer
      cape_udc_set_s_cp (item, bind->buffer);
      
      break;
    }
    case CAPE_UDC_NUMBER:
    {
      number_t* h = bind->buffer;      
      cape_udc_set_n (item, *h);
      
      break;
    }
    case CAPE_UDC_FLOAT:
    {
      double* h = bind->buffer;      
      cape_udc_set_f (item, *h);
      
      break;
    }
    case CAPE_UDC_BOOL:
    {
      number_t* h = bind->buffer;      
      cape_udc_set_b (item, *h);
      
      break;
    }
  }  
}

//------------------------------------------------------------------------------------------------------

int adbl_bindvars_set (AdblBindVars self, CapeUdc item, CapeErr err)
{
  if (self->pos < self->size)
  {
    MYSQL_BIND* bind = &(self->binds[self->pos]);
    
    adbl_bind_set (bind, item);
    
    self->pos++;
  }
  
  return CAPE_ERR_NONE;
}

//------------------------------------------------------------------------------------------------------

int adbl_bindvars_add (AdblBindVars self, CapeUdc item, CapeErr err)
{
  if (self->pos < self->size)
  {
    MYSQL_BIND* bind = &(self->binds[self->pos]);
    
    adbl_bind_add (bind, item);
    
    self->pos++;
  }
  
  return CAPE_ERR_NONE;
}

//------------------------------------------------------------------------------------------------------

void adbl_bindvars_get (AdblBindVars self, number_t index, CapeUdc item)
{
  if (index < self->size)
  {
    MYSQL_BIND* bind = &(self->binds[index]);
    
    adbl_bind_get (bind, item);
  }
}

//------------------------------------------------------------------------------------------------------

int adbl_bindvars_set_from_node (AdblBindVars self, CapeUdc node, CapeErr err)
{
  int res;
  CapeUdcCursor* cursor = cape_udc_cursor_new (node, CAPE_DIRECTION_FORW);
  
  while (cape_udc_cursor_next (cursor))
  {
    const CapeString param_name = cape_udc_name (cursor->item);
    if (param_name)
    {
      res = adbl_bindvars_set (self, cursor->item, err);
      if (res)
      {
        goto exit;
      }
    }
  }
  
  res = CAPE_ERR_NONE;
  
  exit:
  
  cape_udc_cursor_del (&cursor);
  return CAPE_ERR_NONE;
}

//------------------------------------------------------------------------------------------------------

int adbl_bindvars_add_from_node (AdblBindVars self, CapeUdc node, CapeErr err)
{
  int res;
  CapeUdcCursor* cursor = cape_udc_cursor_new (node, CAPE_DIRECTION_FORW);
  
  while (cape_udc_cursor_next (cursor))
  {
    const CapeString column_name = cape_udc_name (cursor->item);
    if (column_name)
    {
      res = adbl_bindvars_add (self, cursor->item, err);
      if (res)
      {
        goto exit;
      }
    }
  }  
  
  res = CAPE_ERR_NONE;
  
  exit:
  
  cape_udc_cursor_del (&cursor);
  return CAPE_ERR_NONE;
}

//------------------------------------------------------------------------------------------------------
