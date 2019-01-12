#include "entc_list.h"

// cape includes
#include "sys/entc_types.h"

//-----------------------------------------------------------------------------

struct EntcListNode_s
{
  
  void* data;
  
  EntcListNode next;
  
  EntcListNode prev;
};

//-----------------------------------------------------------------------------

struct EntcList_s
{
  
  fct_entc_list_onDestroy onDestroy;
  
  EntcListNode fpos;   // first position
  
  EntcListNode lpos;   // last position (for fast add)
  
  unsigned long size;
  
};

//-----------------------------------------------------------------------------

EntcList entc_list_new (fct_entc_list_onDestroy onDestroy)
{
  EntcList self = ENTC_NEW(struct EntcList_s);
  
  self->onDestroy = onDestroy;
  self->fpos = NULL;
  self->lpos = NULL;
  
  self->size = 0;
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_list_clr (EntcList self)
{
  EntcListCursor cursor;
  
  entc_list_cursor_init (self, &cursor, ENTC_DIRECTION_FORW);
  
  while (entc_list_cursor_next (&cursor))
  {
    entc_list_cursor_erase (self, &cursor);
  }
}

//-----------------------------------------------------------------------------

void entc_list_del (EntcList* pself)
{
  EntcList self = *pself;
  
  entc_list_clr (self);
  
  ENTC_DEL(pself, struct EntcList_s);
}

//-----------------------------------------------------------------------------

EntcListNode entc_list_push_back (EntcList self, void* data)
{
  EntcListNode node = ENTC_NEW(struct EntcListNode_s);
  
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

EntcListNode entc_list_push_front (EntcList self, void* data)
{
  EntcListNode node = ENTC_NEW(struct EntcListNode_s);
  
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

unsigned long entc_list_size (EntcList self)
{
  return self->size;
}

//-----------------------------------------------------------------------------

int entc_list_empty (EntcList self)
{
  return self->fpos == NULL;
}

//-----------------------------------------------------------------------------

int entc_list_hasContent (EntcList self)
{
  return self->fpos != NULL;
}

//-----------------------------------------------------------------------------

void* entc_list_node_data (EntcListNode node)
{
  return node->data;
}

//-----------------------------------------------------------------------------

void* entc_list_node_extract (EntcList self, EntcListNode node)
{
  void* ret = NULL;
  
  if (node)
  {
    EntcListNode prev = node->prev;
    EntcListNode next = node->next;
    
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
    
    ENTC_DEL(&node, struct EntcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return ret;
}

//-----------------------------------------------------------------------------

void entc_list_node_erase (EntcList self, EntcListNode node)
{
  void* data = entc_list_node_extract (self, node);
  
  if (data && self->onDestroy)
  {
    self->onDestroy (data);
  }
}

//-----------------------------------------------------------------------------

EntcListNode entc_list_noed_next (EntcListNode node)
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

EntcListNode entc_list_node_begin (EntcList self)
{
  return self->fpos;
}

//-----------------------------------------------------------------------------

void entc_list_node_swap (EntcListNode node1, EntcListNode node2)
{
  if (node1 && node2)
  {
    void* h = node1->data;
    
    node1->data = node2->data;
    node2->data = h;
  }
}

//-----------------------------------------------------------------------------

EntcList entc_list_slice_extract (EntcList self, EntcListNode nodeFrom, EntcListNode nodeTo)
{
  EntcListNode prev = NULL;
  EntcListNode next = NULL;
  
  EntcList slice = ENTC_NEW(struct EntcList_s);
  
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
    EntcListCursor cursor;
    entc_list_cursor_init (slice, &cursor, ENTC_DIRECTION_FORW);
    
    // iterate to find the correct size
    while (entc_list_cursor_next (&cursor))
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

void entc_list_link_prev (EntcList self, EntcListNode node, EntcListNode prev)
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

void entc_list_link_next (EntcList self, EntcListNode node, EntcListNode next)
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

void entc_list_sort (EntcList self, fct_entc_list_onCompare onCompare)
{
  EntcListNode list = self->fpos;
  EntcListNode tail;
  EntcListNode p;
  EntcListNode q;
  EntcListNode e;
  
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

EntcList entc_list_clone (EntcList orig, fct_entc_list_onClone onClone)
{
  EntcList self = ENTC_NEW(struct EntcList_s);
  
  self->onDestroy = orig->onDestroy;
  self->fpos = NULL;
  self->lpos = NULL;
  
  self->size = 0;
  
  {
    EntcListCursor cursor;
    entc_list_cursor_init (orig, &cursor, ENTC_DIRECTION_FORW);
    
    // iterate to find the correct size
    while (entc_list_cursor_next (&cursor))
    {
      void* data = NULL;
      
      if (onClone)  // if not, the value will be null
      {
        data = onClone (entc_list_node_data (cursor.node));
      }
      
      entc_list_push_back(self, data);
    }
  }

  return self;
}

//-----------------------------------------------------------------------------

void entc_list_node_replace (EntcList self, EntcListNode node, void* data)
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

EntcListCursor* entc_list_cursor_create (EntcList self, int direction)
{
  EntcListCursor* cursor = ENTC_NEW(EntcListCursor);
  
  entc_list_cursor_init (self, cursor, direction);
  
  return cursor;
}

//-----------------------------------------------------------------------------

void entc_list_cursor_destroy (EntcListCursor** pcursor)
{
  ENTC_DEL (pcursor, EntcListCursor);
}

//-----------------------------------------------------------------------------

void entc_list_cursor_init (EntcList self, EntcListCursor* cursor, int direction)
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

int entc_list_cursor_next (EntcListCursor* cursor)
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

int entc_list_cursor_prev (EntcListCursor* cursor)
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

void* entc_list_cursor_extract (EntcList self, EntcListCursor* cursor)
{
  EntcListNode node = cursor->node;
  void* ret = NULL;
  
  if (node)
  {
    EntcListNode prev = node->prev;
    EntcListNode next = node->next;
    
    if (prev)
    {
      prev->next = next;
      
      if (cursor->direction == ENTC_DIRECTION_FORW)
      {
        // adjust the cursor
        cursor->node = prev;
      }
    }
    else
    {
      // this was the first node
      self->fpos = next;
      
      if (cursor->direction == ENTC_DIRECTION_FORW)
      {
        // reset cursor
        entc_list_cursor_init (self, cursor, cursor->direction);
      }
    }
    
    if (next)
    {
      next->prev = prev;
      
      if (cursor->direction == ENTC_DIRECTION_PREV)
      {
        // adjust the cursor
        cursor->node = next;
      }
    }
    else
    {
      // this was the last node
      self->lpos = prev;

      if (cursor->direction == ENTC_DIRECTION_PREV)
      {
        // reset cursor
        entc_list_cursor_init (self, cursor, cursor->direction);
      }
    }
    
    ret = node->data;
    
    ENTC_DEL(&node, struct EntcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return ret;
}

//-----------------------------------------------------------------------------

void entc_list_cursor_erase (EntcList self, EntcListCursor* cursor)
{
  void* data = entc_list_cursor_extract (self, cursor);
  
  if (data && self->onDestroy)
  {
    self->onDestroy (data);
  }
}

//-----------------------------------------------------------------------------

void entc_list_insert_slice_flpos (EntcList self, EntcListCursor* cursor, EntcListNode fpos, EntcListNode lpos)
{
  EntcListNode node = cursor->node;
  
  if (cursor->direction == ENTC_DIRECTION_PREV)
  {
    // relink fpos
    entc_list_link_prev (self, fpos, node ? node->prev : self->lpos); // maybe self->lpos is reset
    
    // relink lpos
    entc_list_link_prev (self, node, lpos);   // lpos is always set
  }
  else
  {
    // relink lpos
    entc_list_link_next (self, lpos, node ? node->next : self->fpos); // maybe self->fpos is reset
    
    // relink fpos
    entc_list_link_next (self, node, fpos);   // fpos is always set
  }
}

//-----------------------------------------------------------------------------

void entc_list_slice_insert (EntcList self, EntcListCursor* cursor, EntcList* pslice)
{
  EntcList slice = *pslice;
  if (slice)
  {
    EntcListNode fpos = slice->fpos;
    EntcListNode lpos = slice->lpos;
    
    if (fpos && lpos)
    {
      entc_list_insert_slice_flpos (self, cursor, fpos, lpos);
    }
    
    self->size += slice->size;
    
    // clean
    slice->fpos = NULL;
    slice->lpos = NULL;
    
    entc_list_del (pslice);
  }
}

//-----------------------------------------------------------------------------

void* entc_list_pop_front (EntcList self)
{
  void* data = NULL;
  
  if (self->fpos)
  {
    EntcListNode node = self->fpos;
    EntcListNode next = node->next;
    
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
    
    ENTC_DEL(&node, struct EntcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return data;
}

//-----------------------------------------------------------------------------

void* entc_list_pop_back (EntcList self)
{
  void* data = NULL;
  
  if (self->lpos)
  {
    EntcListNode node = self->lpos;
    EntcListNode prev = node->prev;
    
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
    
    ENTC_DEL(&node, struct EntcListNode_s);
    
    if (self->size > 0)
    {
      self->size--;
    }
  }
  
  return data;
}

//-----------------------------------------------------------------------------
