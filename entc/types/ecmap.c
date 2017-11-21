#include "ecmap.h"

#include "system/macros.h"

//=============================================================================

struct EcMapNode_s
{
  int red;                        // Color red (1), black (0)
  struct EcMapNode_s* link[2];   // Link left [0] and right [1]
  
  void* val;
  void* key;                      // User provided, used indirectly via rb_tree_node_cmp_f.
  
}; typedef struct EcMapNode_s* EcMapNode;

//-----------------------------------------------------------------------------

EcMapNode ecmap_node_create (void* key, void* val)
{
  EcMapNode self = ENTC_NEW (struct EcMapNode_s);
  
  self->red = 1;
  self->link[0] = self->link[1] = NULL;
  
  self->val = val;
  self->key = key;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecmap_node_destroy (EcMapNode* pself)
{
  ENTC_DEL (pself, struct EcMapNode_s);
}

//-----------------------------------------------------------------------------

void* ecmap_node_value (EcMapNode self)
{
  return self->val;
}

//-----------------------------------------------------------------------------

void* ecmap_node_key (EcMapNode self)
{
  return self->key;
}

//-----------------------------------------------------------------------------

void* ecmap_node_extract (EcMapNode self)
{
  void* data = self->val;
  
  self->val = NULL;
  
  return data;
}

//-----------------------------------------------------------------------------

static int ecmap_node_isRed (EcMapNode self)
{
  return self ? self->red : 0;
}

//-----------------------------------------------------------------------------

static EcMapNode ecmap_node_rotate (EcMapNode self, int dir)
{
  EcMapNode result = NULL;
  
  if (self)
  {
    result = self->link[!dir];
    self->link[!dir] = result->link[dir];
    result->link[dir] = self;
    self->red = 1;
    result->red = 0;
  }
  
  return result;
}

//-----------------------------------------------------------------------------

static EcMapNode ecmap_node_rotate2 (EcMapNode self, int dir)
{
  EcMapNode result = NULL;
  
  if (self)
  {
    self->link[!dir] = ecmap_node_rotate (self->link[!dir], !dir);
    result = ecmap_node_rotate (self, dir);
  }
  
  return result;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecmap_node_cmp (const void* a, const void* b)
{
  return strcmp(a, b);
}

//=============================================================================

struct EcMap_s
{
  EcMapNode root;
  
  fct_ecmap_cmp onCompare;
  
  fct_ecmap_destroy onDestroy;
  
  size_t             size;
  
  //void              *info; // User provided, not used by rb_tree
};

//-----------------------------------------------------------------------------

EcMap ecmap_create (fct_ecmap_cmp onCompare, fct_ecmap_destroy onDestroy)
{
  EcMap self = ENTC_NEW (struct EcMap_s);
  
  self->root = NULL;
  self->size = 0;
  
  self->onCompare = onCompare ? onCompare : ecmap_node_cmp;
  self->onDestroy = onDestroy;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecmap_clear (EcMap self)
{
  EcMapNode node = self->root;
  EcMapNode save = NULL;
  
  // Rotate away the left links so that
  // we can treat this like the destruction
  // of a linked list
  while (node)
  {
    if (node->link[0] == NULL)
    {
      // No left links, just kill the node and move on
      save = node->link[1];
      
      // use user defined destroy
      if (self->onDestroy)
      {
        self->onDestroy (node->key, node->val);
      }
      
      ecmap_node_destroy (&node);
    }
    else
    {
      // Rotate away the left link and check again
      save = node->link[0];
      node->link[0] = save->link[1];
      save->link[1] = node;
    }
    
    node = save;
  }
  
  self->root = NULL;
  self->size = 0;
}

//-----------------------------------------------------------------------------

void ecmap_destroy (EcMap* pself)
{
  EcMap self =  *pself;
  
  ecmap_clear (self);
  
  free (self);
  *pself = NULL;
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_find (EcMap self, void* key)
{
  EcMapNode it = self->root;
  
  while (it)
  {
    int cmp = self->onCompare(it->key, key);
    if (cmp)
    {
      // If the tree supports duplicates, they should be
      // chained to the right subtree for this to work
      it = it->link[cmp < 0];
    }
    else
    {
      break;
    }
  }
  
  return it ? it : NULL;
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_insert (EcMap self, void* key, void* val)
{
  EcMapNode node = ecmap_node_create (key, val);
  
  {
    if (self->root == NULL)
    {
      self->root = node;
    }
    else
    {
      struct EcMapNode_s head = { 0 }; // False tree root
      EcMapNode g;                     // Grandparent
      EcMapNode t;                     // parent
      EcMapNode p;                     // Iterator
      EcMapNode q;                     // parent
      
      int dir = 0, last = 0;
      
      // Set up our helpers
      t = &head;
      g = p = NULL;
      q = t->link[1] = self->root;
      
      // Search down the tree for a place to insert
      while (1)
      {
        if (q == NULL)
        {
          // Insert node at the first null link.
          p->link[dir] = q = node;
        }
        else if (ecmap_node_isRed (q->link[0]) && ecmap_node_isRed (q->link[1]))
        {
          // Simple red violation: color flip
          q->red = 1;
          q->link[0]->red = 0;
          q->link[1]->red = 0;
        }
        
        if (ecmap_node_isRed (q) && ecmap_node_isRed (p))
        {
          // Hard red violation: rotations necessary
          int dir2 = t->link[1] == g;
          if (q == p->link[last])
          {
            t->link[dir2] = ecmap_node_rotate (g, !last);
          }
          else
          {
            t->link[dir2] = ecmap_node_rotate2 (g, !last);
          }
        }
        
        // Stop working if we inserted a node. This
        // check also disallows duplicates in the tree
        if (self->onCompare (q->key, node->key) == 0)
        {
          break;
        }
        
        last = dir;
        dir = self->onCompare (q->key, node->key) < 0;
        
        // Move the helpers down
        if (g != NULL)
        {
          t = g;
        }
        
        g = p, p = q;
        q = q->link[dir];
      }
      
      // Update the root (it may be different)
      self->root = head.link[1];
    }
    
    // Make the root black for simplified logic
    self->root->red = 0;
    ++self->size;
  }
  
  return node;
}

//-----------------------------------------------------------------------------

void ecmap_erase (EcMap self, EcMapNode node)
{
  EcMapNode h = ecmap_extract (self, node);
  
  if (self->onDestroy)
  {
    self->onDestroy (h->key, h->val);
  }
  
  // delete
  ecmap_node_destroy (&h);
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_extract (EcMap self, EcMapNode node)
{
  EcMapNode ret = NULL;
  
  if (self->root != NULL)
  {
    struct EcMapNode_s head = {0};   // False tree root
    EcMapNode q;
    EcMapNode p;
    EcMapNode g;                     // Helpers
    EcMapNode f = NULL;              // Found item
    int dir = 1;
    
    // Set up our helpers
    q = &head;
    g = p = NULL;
    q->link[1] = self->root;
    
    // Search and push a red node down
    // to fix red violations as we go
    while (q->link[dir] != NULL)
    {
      int last = dir;
      
      // Move the helpers down
      g = p, p = q;
      q = q->link[dir];
      dir = self->onCompare (q->key, node->key) < 0;
      
      // Save the node with matching value and keep
      // going; we'll do removal tasks at the end
      if (self->onCompare (q->key, node->key) == 0)
      {
        f = q;
      }
      
      // Push the red node down with rotations and color flips
      if (!ecmap_node_isRed(q) && !ecmap_node_isRed(q->link[dir]))
      {
        if (ecmap_node_isRed(q->link[!dir]))
        {
          p = p->link[last] = ecmap_node_rotate (q, dir);
        }
        else if (!ecmap_node_isRed (q->link[!dir]))
        {
          EcMapNode s = p->link[!last];
          if (s)
          {
            if (!ecmap_node_isRed (s->link[!last]) && !ecmap_node_isRed (s->link[last]))
            {
              // Color flip
              p->red = 0;
              s->red = 1;
              q->red = 1;
            }
            else
            {
              int dir2 = g->link[1] == p;
              if (ecmap_node_isRed(s->link[last]))
              {
                g->link[dir2] = ecmap_node_rotate2 (p, last);
              }
              else if (ecmap_node_isRed(s->link[!last]))
              {
                g->link[dir2] = ecmap_node_rotate (p, last);
              }
              
              // Ensure correct coloring
              q->red = g->link[dir2]->red = 1;
              g->link[dir2]->link[0]->red = 0;
              g->link[dir2]->link[1]->red = 0;
            }
          }
        }
      }
    }
    
    // Replace and remove the saved node
    if (f)
    {
      void* h1 = f->key;
      void* h2 = f->val;
      
      f->key = q->key;
      f->val = q->val;
      
      q->key = h1;
      q->val = h2;
      
      p->link[p->link[1] == q] = q->link[q->link[0] == NULL];
      
      ret = q;
      q = NULL;
    }
    
    // Update the root (it may be different)
    self->root = head.link[1];
    
    // Make the root black for simplified logic
    if (self->root != NULL)
    {
      self->root->red = 0;
    }
    
    --self->size;
  }
  
  return ret;
}

//-----------------------------------------------------------------------------

unsigned long ecmap_size (EcMap self)
{
  return self->size;
}

//=============================================================================

EcMap ecmap_clone (EcMap orig, fct_ecmap_onClone onCloneKey, fct_ecmap_onClone onCloneVal)
{
  EcMap self = ecmap_create (orig->onCompare, orig->onDestroy);
  
  EcMapCursor* cursor = ecmap_cursor_create (orig, LIST_DIR_NEXT);

  while (ecmap_cursor_next (cursor))
  {
    if (onCloneKey && onCloneVal)
    {
      void* key = onCloneKey (ecmap_node_key (cursor->node));
      void* val = onCloneVal (ecmap_node_value (cursor->node));
      
      ecmap_insert(self, key, val);
    }
  }

  ecmap_cursor_destroy (&cursor);
  
  return self;
}

//=============================================================================

void ecmap_iterator_start (EcMapCursor* cursor, EcMap self, int dir)
{
  cursor->node = self->root;
  cursor->top = 0;
  
  // Save the path for later selfersal
  if (cursor->node != NULL)
  {
    while (cursor->node->link[dir] != NULL)
    {
      cursor->path[cursor->top++] = cursor->node;
      cursor->node = cursor->node->link[dir];
    }
  }
}

//-----------------------------------------------------------------------------

void cmap_iterator_move (EcMapCursor* cursor, int dir)
{
  if (cursor->node->link[dir] != NULL)
  {
    // Continue down this branch
    cursor->path[cursor->top++] = cursor->node;
    cursor->node = cursor->node->link[dir];
    
    while ( cursor->node->link[!dir] != NULL )
    {
      cursor->path[cursor->top++] = cursor->node;
      cursor->node = cursor->node->link[!dir];
    }
  }
  else
  {
    // Move to the next branch
    EcMapNode last = NULL;
    
    do
    {
      if (cursor->top == 0)
      {
        cursor->node = NULL;
        break;
      }
      
      last = cursor->node;
      cursor->node = cursor->path[--cursor->top];
      
    }
    while (last == cursor->node->link[dir]);
  }
}

//-----------------------------------------------------------------------------

EcMapCursor* ecmap_cursor_create (EcMap self, int direction)
{
  EcMapCursor* cursor = ENTC_NEW (EcMapCursor);
  
  ecmap_cursor_init (self, cursor, direction);
  
  return cursor;
}

//-----------------------------------------------------------------------------

void ecmap_cursor_destroy (EcMapCursor** pcursor)
{
  ENTC_DEL(pcursor, EcMapCursor);
}

//-----------------------------------------------------------------------------

void ecmap_cursor_init (EcMap self, EcMapCursor* cursor, int direction)
{
  // initialize the iterator part
  cursor->tree = self;
  cursor->top = 0;
  cursor->node = NULL;
  
  if (direction)
  {
    ecmap_iterator_start (cursor, self, 0);
  }
  else
  {
    ecmap_iterator_start (cursor, self, 1);
  }
  
  cursor->position = -1;
  cursor->direction = direction;
}

//-----------------------------------------------------------------------------

int ecmap_cursor_next (EcMapCursor* cursor)
{
  if (cursor->position < 0)
  {
    cursor->position = 0;
  }
  else
  {
    cmap_iterator_move (cursor, 1);
    cursor->position++;
  }
  
  return cursor->node != NULL;
}

//-----------------------------------------------------------------------------

int ecmap_cursor_prev (EcMapCursor* cursor)
{
  if (cursor->position < 0)
  {
    cursor->position = 0;
  }
  else
  {
    cmap_iterator_move (cursor, 0);
    cursor->position++;
  }
  
  return cursor->node != NULL;
}

//-----------------------------------------------------------------------------

void ecmap_cursor_erase (EcMap self, EcMapCursor* cursor)
{
  // but cursor.node points to invalid node now
  if (cursor->position < 0)
  {
    if (cursor->node)
    {
      // this will remove the node
      ecmap_erase (self, cursor->node);
    }
    
    if (cursor->direction)
    {
      ecmap_iterator_start (cursor, self, 0);
    }
    else
    {
      ecmap_iterator_start (cursor, self, 1);
    }
  }
  else
  {
    EcMapNode node = cursor->node;
    if (node)
    {
      if (cursor->direction)
      {
        cmap_iterator_move (cursor, 0);
      }
      else
      {
        cmap_iterator_move (cursor, 1);
      }
    
      // this will remove the node
      ecmap_erase (self, node);
    }
    
    if (cursor->node == NULL)
    {
      if (cursor->direction)
      {
        ecmap_iterator_start (cursor, self, 0);
      }
      else
      {
        ecmap_iterator_start (cursor, self, 1);
      }
    }
  }
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_cursor_extract (EcMap self, EcMapCursor* cursor)
{
  EcMapNode ret = NULL;
  
  // but cursor.node points to invalid node now
  if (cursor->position < 0)
  {
    if (cursor->node)
    {
      // this will remove the node
      ecmap_erase (self, cursor->node);
    }
    
    if (cursor->direction)
    {
      ecmap_iterator_start (cursor, self, 0);
    }
    else
    {
      ecmap_iterator_start (cursor, self, 1);
    }
  }
  else
  {
    EcMapNode node = cursor->node;
    if (node)
    {
      if (cursor->direction)
      {
        cmap_iterator_move (cursor, 0);
      }
      else
      {
        cmap_iterator_move (cursor, 1);
      }
      
      // this will remove the node
      ret = ecmap_extract (self, node);
    }
    
    if (cursor->node == NULL)
    {
      if (cursor->direction)
      {
        ecmap_iterator_start (cursor, self, 0);
      }
      else
      {
        ecmap_iterator_start (cursor, self, 1);
      }
    }
  }
  
  return ret;
}

//-----------------------------------------------------------------------------
