
#include "eclist.h"

#include "system/macros.h"

//-----------------------------------------------------------------------------

struct EcListNode_s
{
  
  void* data;
  
  EcListNode next;
  
  EcListNode prev;
};

//-----------------------------------------------------------------------------

struct EcList_s
{
  
  fct_eclist_onDestroy onDestroy;
  
  EcListNode fpos;   // first position
  
  EcListNode lpos;   // last position (for fast add)
  
  unsigned long size;
  
};

//-----------------------------------------------------------------------------

EcList eclist_create (fct_eclist_onDestroy onDestroy)
{
  EcList self = ENTC_NEW(struct EcList_s);
  
  self->onDestroy = onDestroy;
  self->fpos = NULL;
  self->lpos = NULL;
  
  self->size = 0;
  
  return self;
}

//-----------------------------------------------------------------------------

void eclist_clear (EcList self)
{
  EcListCursor cursor;
  
  eclist_cursor_init (self, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    eclist_erase (self, &cursor);
  }
}

//-----------------------------------------------------------------------------

void eclist_destroy (EcList* pself)
{
  EcList self = *pself;
  
  eclist_clear (self);
  
  ENTC_DEL(pself, struct EcList_s);
}

//-----------------------------------------------------------------------------

EcListNode eclist_push_back (EcList self, void* data)
{
  EcListNode node = ENTC_NEW(struct EcListNode_s);
  
  node->data = data;
  
  node->next = NULL; // last element
  node->prev = self->lpos;
  
  // assign next node at last element
  if (self->lpos)
  {
    self->lpos->next = node;
  }
  
  // set new last element
  self->lpos = node;
  
  // if we don't have a first element, set it now
  if (self->fpos == NULL)
  {
    self->fpos = node;
  }
  
  self->size++;
  
  return node;
}

//-----------------------------------------------------------------------------

EcListNode eclist_push_front (EcList self, void* data)
{
  EcListNode node = ENTC_NEW(struct EcListNode_s);
  
  node->data = data;
  
  node->next = self->fpos;
  node->prev = NULL;  // first element
  
  // assign next node at first element
  if (self->fpos)
  {
    self->fpos->prev = node;
  }
  
  // set new last element
  self->fpos = node;
  
  // if we don't have a last element, set it now
  if (self->lpos == NULL)
  {
    self->lpos = node;
  }
  
  self->size++;
  
  return node;
}

//-----------------------------------------------------------------------------

unsigned long eclist_size (EcList self)
{
  return self->size;
}

//-----------------------------------------------------------------------------

void* eclist_data (EcListNode node)
{
  return node->data;
}

//-----------------------------------------------------------------------------

EcListNode eclist_next (EcListNode node)
{
  if (node)
  {
    return node->next;
  }
  else
  {
    return NULL;
  }
}

//-----------------------------------------------------------------------------

void eclist_replace (EcList self, EcListNode node, void* data)
{
  if (node)
  {
    if (self->onDestroy)
    {
      self->onDestroy (node->data);
    }
    
    node->data = data;
  }
}

//-----------------------------------------------------------------------------

EcListCursor* eclist_cursor_create (EcList self, int direction)
{
  EcListCursor* cursor = ENTC_NEW(EcListCursor);
  
  eclist_cursor_init (self, cursor, direction);
  
  return cursor;
}

//-----------------------------------------------------------------------------

void eclist_cursor_destroy (EcListCursor** pcursor)
{
  ENTC_DEL (pcursor, EcListCursor);
}

//-----------------------------------------------------------------------------

void eclist_cursor_init (EcList self, EcListCursor* cursor, int direction)
{
  if (direction)
  {
    cursor->node = self->fpos;
  }
  else
  {
    cursor->node = self->lpos;
  }
  
  cursor->position = -1;
  cursor->direction = direction;
}

//-----------------------------------------------------------------------------

int eclist_cursor_next (EcListCursor* cursor)
{
  if (cursor->position < 0)
  {
    cursor->position = 0;
    return cursor->node != NULL;
  }
  else
  {
    if (cursor->node)
    {
      // advance position
      cursor->position++;
      
      // grap next node
      cursor->node = cursor->node->next;
      return cursor->node != NULL;
    }
    else
    {
      return 0;
    }
  }
}

//-----------------------------------------------------------------------------

int eclist_cursor_prev (EcListCursor* cursor)
{
  if (cursor->position < 0)
  {
    cursor->position = 0;
    return cursor->node != NULL;
  }
  else
  {
    if (cursor->node)
    {
      // advance position
      cursor->position++;
      
      // grap next node
      cursor->node = cursor->node->prev;
      return cursor->node != NULL;
    }
    else
    {
      return 0;
    }
  }
}

//-----------------------------------------------------------------------------

void* eclist_extract (EcList self, EcListCursor* cursor)
{
  EcListNode node = cursor->node;
  void* ret = NULL;
  
  if (node)
  {
    EcListNode prev = node->prev;
    EcListNode next = node->next;
    
    if (prev)
    {
      prev->next = next;
      
      if (cursor->direction == LIST_DIR_NEXT)
      {
        // adjust the cursor
        cursor->node = prev;
      }
    }
    else
    {
      // this was the first node
      self->fpos = next;
      
      if (cursor->direction == LIST_DIR_NEXT)
      {
        // reset cursor
        eclist_cursor_init (self, cursor, cursor->direction);
      }
    }
    
    if (next)
    {
      next->prev = prev;
      
      if (cursor->direction == LIST_DIR_PREV)
      {
        // adjust the cursor
        cursor->node = next;
      }
    }
    else
    {
      // this was the last node
      self->lpos = prev;

      if (cursor->direction == LIST_DIR_PREV)
      {
        // reset cursor
        eclist_cursor_init (self, cursor, cursor->direction);
      }
    }
    
    ret = node->data;
    
    ENTC_DEL(&node, struct EcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return ret;
}

//-----------------------------------------------------------------------------

void eclist_erase (EcList self, EcListCursor* cursor)
{
  void* data = eclist_extract (self, cursor);
  
  if (data && self->onDestroy)
  {
    self->onDestroy (data);
  }
}

//-----------------------------------------------------------------------------

void* eclist_pop_front (EcList self)
{
  void* data = NULL;
  
  if (self->fpos)
  {
    EcListNode node = self->fpos;
    EcListNode next = node->next;
    
    if (next)
    {
      next->prev = NULL;
      self->fpos = next;
    }
    else
    {
      // this was the last node
      self->lpos = NULL;
      self->fpos = NULL;
    }
    
    data = node->data;
    
    ENTC_DEL(&node, struct EcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return data;
}

//-----------------------------------------------------------------------------

void* eclist_pop_back (EcList self)
{
  void* data = NULL;
  
  if (self->lpos)
  {
    EcListNode node = self->lpos;
    EcListNode prev = node->prev;
    
    if (prev)
    {
      prev->next = NULL;
      self->lpos = prev;
    }
    else
    {
      // this was the last node
      self->lpos = NULL;
      self->fpos = NULL;
    }
    
    data = node->data;
    
    ENTC_DEL(&node, struct EcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return data;
}

//-----------------------------------------------------------------------------
