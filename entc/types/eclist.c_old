#include "eclist.h"

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
    eclist_cursor_erase (self, &cursor);
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

int eclist_empty (EcList self)
{
  return self->fpos == NULL;
}

//-----------------------------------------------------------------------------

int eclist_hasContent (EcList self)
{
  return self->fpos != NULL;
}

//-----------------------------------------------------------------------------

void* eclist_data (EcListNode node)
{
  return node->data;
}

//-----------------------------------------------------------------------------

void* eclist_extract (EcList self, EcListNode node)
{
  void* ret = NULL;
  
  if (node)
  {
    EcListNode prev = node->prev;
    EcListNode next = node->next;
    
    if (prev)
    {
      prev->next = next;
    }
    else
    {
      // this was the first node
      self->fpos = next;
    }
    
    if (next)
    {
      next->prev = prev;
    }
    else
    {
      // this was the last node
      self->lpos = prev;
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

void eclist_erase (EcList self, EcListNode node)
{
  void* data = eclist_extract (self, node);
  
  if (data && self->onDestroy)
  {
    self->onDestroy (data);
  }
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

EcListNode eclist_begin (EcList self)
{
  return self->fpos;
}

//-----------------------------------------------------------------------------

EcList eclist_slice (EcList self, EcListNode nodeFrom, EcListNode nodeTo)
{
  EcListNode prev = NULL;
  EcListNode next = NULL;
  
  EcList slice = ENTC_NEW(struct EcList_s);
  
  slice->onDestroy = self->onDestroy;
  slice->fpos = NULL;
  slice->lpos = NULL;
  
  slice->size = 0;
  
  if (nodeFrom)
  {
    prev = nodeFrom->prev;
    nodeFrom->prev = NULL;      // terminate
    
    slice->fpos = nodeFrom;
  }
  else
  {
    slice->fpos = self->fpos;   // use first node
  }
  
  if (nodeTo)
  {
    next = nodeTo->next;
    nodeTo->next = NULL;        // terminate
    
    slice->lpos = nodeTo;
  }
  else
  {
    slice->lpos = self->lpos;   // use last node
  }
  
  // count elements
  if ((slice->fpos == self->fpos)&&(slice->lpos == self->lpos))
  {
    slice->size = self->size;
  }
  else
  {
    EcListCursor cursor;
    eclist_cursor_init (slice, &cursor, LIST_DIR_NEXT);
    
    // iterate to find the correct size
    while (eclist_cursor_next (&cursor))
    {
      slice->size++;
    }
  }
  
  self->size = self->size - slice->size;
  
  // cut out
  if (prev)
  {
    prev->next = next;
  }
  else
  {
    self->fpos = next;
  }
  
  if (next)
  {
    next->prev = prev;
  }
  else
  {
    self->lpos = prev;
  }
  
  return slice;
}

//-----------------------------------------------------------------------------

void eclist_link_prev (EcList self, EcListNode node, EcListNode prev)
{
  if (prev)
  {
    if (node)
    {
      node->prev = prev;
    }
    else
    {
      self->lpos = prev;
    }
    
    prev->next = node;
  }
  else
  {
    if (node)
    {
      node->prev = NULL;
    }
    else
    {
      // this should never happen
    }
    
    self->fpos = node;
  }
}

//-----------------------------------------------------------------------------

void eclist_link_next (EcList self, EcListNode node, EcListNode next)
{
  if (next)
  {
    if (node)
    {
      node->next = next;
    }
    else
    {
      self->fpos = next;
    }
    
    next->prev = node;
  }
  else
  {
    if (node)
    {
      node->next = NULL;
    }
    else
    {
      // this should never happen
    }
    
    self->lpos = node;
  }
}

//-----------------------------------------------------------------------------

void eclist_swap (EcListNode node1, EcListNode node2)
{
  if (node1 && node2)
  {
    void* h = node1->data;
    
    node1->data = node2->data;
    node2->data = h;
  }
}

//-----------------------------------------------------------------------------

void eclist_sort (EcList self, fct_eclist_onCompare onCompare)
{
  EcListNode list = self->fpos;
  EcListNode tail;
  EcListNode p;
  EcListNode q;
  EcListNode e;
  
  int insize = 1;
  int psize, qsize;
  int nmerges;
  
  // do some prechecks
  if (onCompare == NULL)
  {
    return;
  }
  
  if (list == NULL)
  {
    return;
  }
  
  while (1)
  {
    p = list;
    
    list = NULL;
    tail = NULL;
    
    nmerges = 0;
    
    while (p)
    {
      nmerges++;
      
      q = p;
      psize = 0;
      
      {
        int i;
        for (i = 0; i < insize; i++)
        {
          psize++;
          q = q->next;
          
          if (!q) break;
        }
      }
      
      qsize = insize;
      
      while (psize > 0 || (qsize > 0 && q))
      {
        if (psize == 0)
        {
          e = q;
          q = q->next;
          qsize--;
        }
        else if (qsize == 0 || !q)
        {
          e = p;
          p = p->next;
          psize--;
        }
        else if (onCompare(p->data, q->data) <= 0)
        {
          e = p;
          p = p->next;
          psize--;
        }
        else
        {
          e = q;
          q = q->next;
          qsize--;
        }
        
        if (tail)
        {
          tail->next = e;
        }
        else
        {
          list = e;
        }
        
        e->prev = tail;
        tail = e;
      }
      
      p = q;
    }
    
    tail->next = NULL;
    
    if (nmerges <= 1)
    {
      self->fpos = list;
      self->lpos = tail;
      return;
    }
    
    insize *= 2;
  }
}

//-----------------------------------------------------------------------------

EcList eclist_clone (EcList orig, fct_eclist_onClone onClone)
{
  EcList self = ENTC_NEW(struct EcList_s);
  
  self->onDestroy = orig->onDestroy;
  self->fpos = NULL;
  self->lpos = NULL;
  
  self->size = 0;
  
  {
    EcListCursor cursor;
    eclist_cursor_init (orig, &cursor, LIST_DIR_NEXT);
    
    // iterate to find the correct size
    while (eclist_cursor_next (&cursor))
    {
      void* data = NULL;
      
      if (onClone)  // if not, the value will be null
      {
        data = onClone (eclist_data (cursor.node));
      }
      
      eclist_push_back(self, data);
    }
  }

  return self;
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

void* eclist_cursor_extract (EcList self, EcListCursor* cursor)
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

void eclist_cursor_erase (EcList self, EcListCursor* cursor)
{
  void* data = eclist_cursor_extract (self, cursor);
  
  if (data && self->onDestroy)
  {
    self->onDestroy (data);
  }
}

//-----------------------------------------------------------------------------

void eclist_insert_slice_flpos (EcList self, EcListCursor* cursor, EcListNode fpos, EcListNode lpos)
{
  EcListNode node = cursor->node;
  
  if (cursor->direction == LIST_DIR_PREV)
  {
    // relink fpos
    eclist_link_prev (self, fpos, node ? node->prev : self->lpos); // maybe self->lpos is reset
    
    // relink lpos
    eclist_link_prev (self, node, lpos);   // lpos is always set
  }
  else
  {
    // relink lpos
    eclist_link_next (self, lpos, node ? node->next : self->fpos); // maybe self->fpos is reset
    
    // relink fpos
    eclist_link_next (self, node, fpos);   // fpos is always set
  }
}

//-----------------------------------------------------------------------------

void eclist_insert_slice (EcList self, EcListCursor* cursor, EcList* pslice)
{
  EcList slice = *pslice;
  if (slice)
  {
    EcListNode fpos = slice->fpos;
    EcListNode lpos = slice->lpos;
    
    if (fpos && lpos)
    {
      eclist_insert_slice_flpos (self, cursor, fpos, lpos);
    }
    
    self->size += slice->size;
    
    // clean
    slice->fpos = NULL;
    slice->lpos = NULL;
    
    eclist_destroy (pslice);
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
