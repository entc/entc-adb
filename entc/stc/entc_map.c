/*
 Copyright (c) 2019 Alexander Kalkhof
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "entc_map.h"

// entc includes
#include "sys/entc_types.h"

//=============================================================================

#define ENTC_MAP_RED     1
#define ENTC_MAP_BLACK   0

#define ENTC_MAP_LEFT    0
#define ENTC_MAP_RIGHT   1

static const int ENTC_MAP_INV[2] = {ENTC_MAP_RIGHT, ENTC_MAP_LEFT};

//-----------------------------------------------------------------------------

struct EntcMapNode_s
{
  int color;
  
  EntcMapNode parent;
  EntcMapNode link[2];
  
  // content
  void* val;
  void* key;
};

//-----------------------------------------------------------------------------

EntcMapNode entc_map_node_new (EntcMapNode parent, void* key, void* val)
{
  EntcMapNode self = ENTC_NEW(struct EntcMapNode_s);
  
  self->color = ENTC_MAP_RED;
  
  self->parent = parent;
  self->link[ENTC_MAP_LEFT] = NULL;
  self->link[ENTC_MAP_RIGHT] = NULL;
  
  self->val = val;
  self->key = key;
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_map_node_del (EntcMapNode* pself)
{
  ENTC_DEL(pself, struct EntcMapNode_s);
}

//-----------------------------------------------------------------------------

void* entc_map_node_value (EntcMapNode self)
{
  return self->val;
}

//-----------------------------------------------------------------------------

void* entc_map_node_key (EntcMapNode self)
{
  return self->key;
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_node_left (EntcMapNode n)
{
  return n == NULL ? NULL : n->link[ENTC_MAP_LEFT];
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_node_right (EntcMapNode n)
{
  return n == NULL ? NULL : n->link[ENTC_MAP_RIGHT];
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_node_toTheLast (EntcMapNode n, int dir)
{
  if (n)
  {
    while (n->link[dir])
    {
      n = n->link[dir];
    }
  }
  
  return n;
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_node_next (EntcMapNode n, int dir)
{
  EntcMapNode m = n->link[dir];
  EntcMapNode p = n->parent;
  
  if (m)
  {
    return entc_map_node_toTheLast (m, ENTC_MAP_INV[dir]);
  }
  
  while (p && p->link[dir] == n)
  {
    n = p;
    p = n->parent;
  }
  
  if (p)
  {
    return p;
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_siblingOf (EntcMapNode n)
{
  return (n == NULL || n->parent == NULL) ? NULL : (n == n->parent->link[ENTC_MAP_LEFT] ? n->parent->link[ENTC_MAP_RIGHT] : n->parent->link[ENTC_MAP_LEFT]);
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_grandparentOf (EntcMapNode n)
{
  return (n == NULL || n->parent == NULL) ? NULL : n->parent->parent;
}

//-----------------------------------------------------------------------------

int entc_map_isRed (EntcMapNode n)
{
  return n && n->color == ENTC_MAP_RED;
}

//-----------------------------------------------------------------------------

int entc_map_isBlack (EntcMapNode n)
{
  return n && n->color == ENTC_MAP_BLACK;
}

//-----------------------------------------------------------------------------

void entc_map_setColor (EntcMapNode n, int color)
{
  if (n)
  {
    n->color = color;
  }
}

//=============================================================================

struct EntcMap_s
{
  EntcMapNode root;
  
  fct_entc_map_cmp onCompare;
  
  fct_entc_map_destroy onDestroy;
  
  size_t size;
};

//-----------------------------------------------------------------------------

static int __STDCALL entc_map_node_cmp (const void* a, const void* b)
{
  const char* s1 = a;
  const char* s2 = b;
  
  return strcmp(s1, s2);
}

//-----------------------------------------------------------------------------

void entc_map_rotate (EntcMap self, EntcMapNode x, int dir)
{
  EntcMapNode y = x->link[ENTC_MAP_INV[dir]];
  if (y)
  {
    EntcMapNode h = y->link[dir];
    EntcMapNode p = x->parent;
    
    // turn y's left subtree into x's right subtree
    x->link[ENTC_MAP_INV[dir]] = h;
    y->link[dir] = x;
    
    if (h)
    {
      h->parent = x;
    }
    
    // link parent
    y->parent = x->parent;
    x->parent = y;
    
    // link parent's link
    if (p)
    {
      // set parents link
      if (p->link[ENTC_MAP_LEFT] == x)
      {
        p->link[ENTC_MAP_LEFT] = y;
      }
      else
      {
        p->link[ENTC_MAP_RIGHT] = y;
      }
    }
    else  // x was root node
    {
      self->root = y;
    }
  }
}

//-----------------------------------------------------------------------------

EntcMap entc_map_new (fct_entc_map_cmp onCompare, fct_entc_map_destroy onDestroy)
{
  EntcMap self = ENTC_NEW(struct EntcMap_s);
  
  self->root = NULL;
  self->size = 0;
  
  self->onCompare = onCompare ? onCompare : entc_map_node_cmp;
  self->onDestroy = onDestroy;
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_map_del_node (EntcMap self, EntcMapNode* pself)
{
  EntcMapNode n = *pself;
  
  if (self->onDestroy)
  {
    self->onDestroy (n->key, n->val);
  }
  
  entc_map_node_del (pself);
}

//-----------------------------------------------------------------------------

void entc_map_clr (EntcMap self)
{
  EntcMapNode n = self->root;
  
  while (n)
  {
    EntcMapNode p;
    
    if (n->link[ENTC_MAP_LEFT])
    {
      p = n->link[ENTC_MAP_LEFT];
      
      // mark that we already went this way
      n->link[ENTC_MAP_LEFT] = NULL;
      
      n = p;
      
      continue;
    }
    
    if (n->link[ENTC_MAP_RIGHT])
    {
      p = n->link[ENTC_MAP_RIGHT];
      
      // mark that we already went this way
      n->link[ENTC_MAP_RIGHT] = NULL;
      
      n = p;
      
      continue;
    }
    
    // no links available -> delete
    p = n->parent;
    
    entc_map_del_node (self, &n);
    
    n = p;
  }
  
  self->root = NULL;
  self->size = 0;
}

//-----------------------------------------------------------------------------

void entc_map_del (EntcMap* pself)
{
  EntcMap self =  *pself;
  
  entc_map_clr (self);
  
  ENTC_DEL(pself, struct EntcMap_s);
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_find (EntcMap self, void* key)
{
  EntcMapNode n = self->root;
  
  while (n)
  {
    int res = self->onCompare (n->key, key);
    
    if (res < 0)
    {
      n = n->link[ENTC_MAP_RIGHT];
    }
    else if (res > 0)
    {
      n = n->link[ENTC_MAP_LEFT];
    }
    else
    {
      return n;
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------

void entc_map_node_swap (EntcMapNode n1, EntcMapNode n2)
{
  void* key = n1->key;
  void* val = n1->val;
  
  n1->key = n2->key;
  n1->val = n2->val;
  
  n2->key = key;
  n2->val = val;
}

//-----------------------------------------------------------------------------

EntcMapNode stdbst_insert (EntcMap self, EntcMapNode z)
{
  EntcMapNode y = NULL;
  EntcMapNode x = self->root;
  
  while (x)
  {
    int res;
    
    y = x;
    
    res = self->onCompare (z->key, x->key);
    if (res < 0)
    {
      x = x->link[ENTC_MAP_LEFT];
    }
    else if (res > 0)
    {
      x = x->link[ENTC_MAP_RIGHT];
    }
    else
    {
      entc_map_node_swap (x, z);
      return x;
    }
  }
  
  z->parent = y;
  
  if (y)
  {
    int res = self->onCompare (z->key, y->key);
    if (res < 0)
    {
      y->link[ENTC_MAP_LEFT] = z;
    }
    else if (res > 0)
    {
      y->link[ENTC_MAP_RIGHT] = z;
    }
    else
    {
      entc_map_node_swap (y, z);
      return y;
    }
  }
  else
  {
    self->root = z;
  }
  
  self->size++;
  
  return z;
}

//-----------------------------------------------------------------------------

void entc_map_insert_balance (EntcMap self, EntcMapNode x)
{
  EntcMapNode n = x;
  n->color = ENTC_MAP_RED;
  
  while (n && entc_map_isRed (n->parent))
  {
    EntcMapNode p = n->parent;   // p is always valid
    EntcMapNode s = entc_map_siblingOf (p);
    
    if (entc_map_isRed (s))
    {
      p->color = ENTC_MAP_BLACK;
      s->color = ENTC_MAP_BLACK;
      
      s->parent->color = ENTC_MAP_RED;
      n = s->parent;
    }
    else
    {
      EntcMapNode g = p->parent;
      if (g)
      {
        if (p == g->link[ENTC_MAP_LEFT])
        {
          if (n == p->link[ENTC_MAP_RIGHT])
          {
            n = p;
            entc_map_rotate (self, n, ENTC_MAP_LEFT);
          }
          
          // reset
          p = n->parent;   // p is always valid
          g = p->parent;   // might be NULL
          
          p->color = ENTC_MAP_BLACK;
          
          if (g)
          {
            g->color = ENTC_MAP_RED;
            entc_map_rotate (self, g, ENTC_MAP_RIGHT);
          }
        }
        else
        {
          if (n == p->link[ENTC_MAP_LEFT])
          {
            n = p;
            entc_map_rotate (self, n, ENTC_MAP_RIGHT);
          }
          
          // reset
          p = n->parent;   // p is always valid
          g = p->parent;   // might be NULL
          
          p->color = ENTC_MAP_BLACK;
          
          if (g)
          {
            g->color = ENTC_MAP_RED;
            entc_map_rotate (self, g, ENTC_MAP_LEFT);
          }
        }
      }
      else
      {
        p->color = ENTC_MAP_BLACK;
      }
    }
  }
  
  self->root->color = ENTC_MAP_BLACK;
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_insert (EntcMap self, void* key, void* val)
{
  EntcMapNode n = entc_map_node_new (NULL, key, val);
  
  if (stdbst_insert (self, n) == n)
  {
    // re-balance the tree
    entc_map_insert_balance (self, n);
  }
  else
  {
    entc_map_del_node (self, &n);
  }
  
  return n;
}

//-----------------------------------------------------------------------------

int entc_map_maxdepth (EntcMapNode n)
{
  int depth = 0;
  
  if (n)
  {
    int dp = 1;
    int dl = 0;
    int dr = 0;
    
    if (n->link[ENTC_MAP_LEFT])
    {
      dl = entc_map_maxdepth (n->link[ENTC_MAP_LEFT]);
    }
    
    if (n->link[ENTC_MAP_RIGHT])
    {
      dr = entc_map_maxdepth (n->link[ENTC_MAP_LEFT]);
    }
    
    if (dr > dl)
    {
      return dp + dr;
    }
    else
    {
      return dp + dl;
    }
  }
  
  return depth;
}

//-----------------------------------------------------------------------------

int entc_map_maxtreedepth (EntcMap self)
{
  return entc_map_maxdepth (self->root);
}

//-----------------------------------------------------------------------------

void entc_map_tree_remove_balance (EntcMap self, EntcMapNode x)
{
  EntcMapNode p = x->parent;
  EntcMapNode s;
  int d;
  
  if (p == NULL)
  {
    // Case 1 - node is the new root.
    
    return;
  }
  
  // get sibling
  if (p->link[ENTC_MAP_LEFT] == x)
  {
    d = ENTC_MAP_RIGHT;
  }
  else
  {
    d = ENTC_MAP_LEFT;
  }
  
  s = p->link[d];
  
  if (s)
  {
    if (s->color == ENTC_MAP_RED)
    {
      // Case 2 - sibling is red.
      
      p->color = ENTC_MAP_RED;
      s->color = ENTC_MAP_BLACK;
      
      entc_map_rotate (self, p, ENTC_MAP_INV[d]);
      
      // Rotation, need to update parent/sibling
      //entc_map_rotate (self, p, d);
      
      p = x->parent;
      
      if (p)
      {
        // get sibling
        if (p->link[ENTC_MAP_LEFT] == x)
        {
          d = ENTC_MAP_RIGHT;
        }
        else
        {
          d = ENTC_MAP_LEFT;
        }
        
        s = p->link[d];
      }
      else
      {
        s = NULL;
      }
    }
    
    if (s && (s->color == ENTC_MAP_BLACK))
    {
      if (entc_map_isBlack (s->link[ENTC_MAP_RIGHT]) && entc_map_isBlack (s->link[ENTC_MAP_LEFT]))
      {
        if (p->color == ENTC_MAP_BLACK)
        {
          // Case 3 - parent, sibling, and sibling's children are black.
          s->color = ENTC_MAP_RED;
          
          entc_map_tree_remove_balance (self, p);
          
          return;
        }
        else
        {
          // Case 4 - sibling and sibling's children are black, but parent is red.
          s->color = ENTC_MAP_RED;
          p->color = ENTC_MAP_BLACK;
          
          return;
        }
      }
      else
      {
        // Case 5 - sibling is black, sibling's left child is red,
        // sibling's right child is black, and node is the left child of
        // its parent.
        if ((x == p->link[ENTC_MAP_LEFT]) && entc_map_isRed (s->link[ENTC_MAP_LEFT]) && entc_map_isBlack (s->link[ENTC_MAP_RIGHT]))
        {
          s->color = ENTC_MAP_RED;
          
          entc_map_rotate (self, p, ENTC_MAP_RIGHT);
          
          p = x->parent;
          
          if (p)
          {
            // get sibling
            if (p->link[ENTC_MAP_LEFT] == x)
            {
              d = ENTC_MAP_RIGHT;
            }
            else
            {
              d = ENTC_MAP_LEFT;
            }
            
            s = p->link[d];
          }
          else
          {
            s = NULL;
          }
        }
        else if ((x == p->link[ENTC_MAP_RIGHT]) && entc_map_isBlack (s->link[ENTC_MAP_LEFT]) && entc_map_isRed (s->link[ENTC_MAP_RIGHT]))
        {
          s->color = ENTC_MAP_RED;
          
          entc_map_rotate (self, p, ENTC_MAP_LEFT);
          
          p = x->parent;
          
          if (p)
          {
            // get sibling
            if (p->link[ENTC_MAP_LEFT] == x)
            {
              d = ENTC_MAP_RIGHT;
            }
            else
            {
              d = ENTC_MAP_LEFT;
            }
            
            s = p->link[d];
          }
          else
          {
            s = NULL;
          }
        }
      }
    }
    
    if (s)
    {
      // Case 6 - sibling is black, sibling's right child is red, and node
      // is the left child of its parent.
      s->color = p->color;
      p->color = ENTC_MAP_BLACK;
      
      if (x == p->link[ENTC_MAP_LEFT])
      {
        entc_map_setColor (s->link[ENTC_MAP_RIGHT], ENTC_MAP_BLACK);
        
        entc_map_rotate (self, p, ENTC_MAP_LEFT);
      }
      else
      {
        entc_map_setColor (s->link[ENTC_MAP_LEFT], ENTC_MAP_BLACK);
        
        entc_map_rotate (self, p, ENTC_MAP_RIGHT);
      }
    }
    
  }
  
  /*
   * 
   * 
   *   while (p && x->color == ENTC_MAP_BLACK)
   *   {
   *      int d;
   *      EntcMapNode s;
   * 
   *      if (p->link[ENTC_MAP_LEFT] == x)
   *      {
   *         d = ENTC_MAP_LEFT;
}
else
{
d = ENTC_MAP_RIGHT;
}

s = p->link[ENTC_MAP_INV[d]];

if (s)
{
if (s->color == ENTC_MAP_RED)
{
s->color = ENTC_MAP_BLACK;
p->color = ENTC_MAP_RED;

entc_map_rotate (self, p, d);

p = x->parent;
s = p->link[ENTC_MAP_INV[d]];
}

if ((s->link[ENTC_MAP_LEFT]->color == ENTC_MAP_BLACK) && (s->link[ENTC_MAP_RIGHT]->color == ENTC_MAP_BLACK))
{
s->color = ENTC_MAP_RED;
x = x->parent;
}
else
{
if (s->link[ENTC_MAP_INV[d]]->color == ENTC_MAP_BLACK)
{
s->link[ENTC_MAP_LEFT]->color = ENTC_MAP_BLACK;
s->color = ENTC_MAP_RED;

entc_map_rotate (self, s, ENTC_MAP_INV[d]);

s = x->parent->link[ENTC_MAP_INV[d]];
}

s->color = x->parent->color;
x->parent->color = ENTC_MAP_BLACK;
s->link[ENTC_MAP_INV[d]]->color = ENTC_MAP_BLACK;

entc_map_rotate (self, x->parent, d);
x = self->root;
}
}
else
{
break;
}
}
*/
}

//-----------------------------------------------------------------------------

void stdbst_replace (EntcMapNode a, EntcMapNode b)
{
  if (a != b)
  {
    int color;
    
    // swap content of nodes
    entc_map_node_swap (a, b);
    
    // swap the color
    color = a->color;
    a->color = b->color;
    b->color = color;
  }
}

//-----------------------------------------------------------------------------

EntcMapNode stdbst_delete (EntcMap self, EntcMapNode x)
{
  EntcMapNode n = x;
  EntcMapNode p;
  EntcMapNode c = NULL;
  EntcMapNode l = n->link[ENTC_MAP_LEFT];
  EntcMapNode r = n->link[ENTC_MAP_RIGHT];
  
  while (l && r)
  {
    if (r)
    {
      n = entc_map_node_next (n, ENTC_MAP_RIGHT);  // successor of n
    }
    else
    {
      n = entc_map_node_next (n, ENTC_MAP_LEFT);  // predecessor of n
    }
    
    l = n->link[ENTC_MAP_LEFT];
    r = n->link[ENTC_MAP_RIGHT];
  }
  
  stdbst_replace (n, x);
  
  if (l)
  {
    c = l;
    
  }
  else if (r)
  {
    c = r;
  }
  
  if (c)
  {
    stdbst_replace (n, c);
    
    n->link[ENTC_MAP_LEFT] = c->link[ENTC_MAP_LEFT];
    
    if (n->link[ENTC_MAP_LEFT])
    {
      n->link[ENTC_MAP_LEFT]->parent = n;
    }
    
    n->link[ENTC_MAP_RIGHT] = c->link[ENTC_MAP_RIGHT];
    
    if (n->link[ENTC_MAP_RIGHT])
    {
      n->link[ENTC_MAP_RIGHT]->parent = n;
    }
    
    if (n->color == ENTC_MAP_BLACK)
    {
      if (c->color == ENTC_MAP_RED)
      {
        c->color = ENTC_MAP_BLACK;
      }
      else
      {
        
      }
    }
    
    n = c;
  }
  else
  {
    p = n->parent;
    
    if (p)
    {
      // find on which link the node is connected
      if (p->link[ENTC_MAP_LEFT] == n)
      {
        p->link[ENTC_MAP_LEFT] = NULL;
      }
      else
      {
        p->link[ENTC_MAP_RIGHT] = NULL;
      }
    }
    else
    {
      self->root = NULL;  // last node
    }
  }
  
  self->size--;
  
  return n;
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_extract (EntcMap self, EntcMapNode node)
{
  if (node)
  {
    EntcMapNode x = stdbst_delete (self, node);
    
    /*
     *      if (node->color == ENTC_MAP_BLACK)
     *      {
     *         // re-balance the tree
     *         entc_map_tree_remove_balance (self, x);
  }
  */
    
    return x;
  }
  
  return node;
}

//-----------------------------------------------------------------------------

void entc_map_erase (EntcMap self, EntcMapNode node)
{
  if (node)
  {
    EntcMapNode node2 = entc_map_extract (self, node);
    
    entc_map_del_node (self, &node2);
  }
}

//-----------------------------------------------------------------------------

unsigned long entc_map_size (EntcMap self)
{
  return self->size;
}

//-----------------------------------------------------------------------------

int entc_map_validate_parent (EntcMapNode n, EntcMapNode parent)
{
  if (n == NULL)
  {
    return 1;
  }
  
  if (n->parent != parent)
  {
    return 0;
  }
  
  if (entc_map_validate_parent (n->link[ENTC_MAP_LEFT], n) == 0)
  {
    return 0;
  }
  
  if (entc_map_validate_parent (n->link[ENTC_MAP_RIGHT], n) == 0)
  {
    return 0;
  }
  
  return 1;
}

//-----------------------------------------------------------------------------

int entc_map_validate (EntcMap self)
{
  return entc_map_validate_parent (self->root, NULL);
}

//=============================================================================

void entc_map_cursor_init (EntcMap self, EntcMapCursor* cursor, int direction)
{
  if (direction == ENTC_DIRECTION_FORW)
  {
    cursor->node = entc_map_node_toTheLast (self->root, ENTC_MAP_LEFT);
  }
  else
  {
    cursor->node = entc_map_node_toTheLast (self->root, ENTC_MAP_RIGHT);
  }
  
  cursor->position = -1;
  cursor->direction = direction;
}

//-----------------------------------------------------------------------------

EntcMapCursor* entc_map_cursor_create (EntcMap self, int direction)
{
  EntcMapCursor* cursor = (EntcMapCursor*)malloc (sizeof(EntcMapCursor));
  
  entc_map_cursor_init (self, cursor, direction);
  
  return cursor;
}

//-----------------------------------------------------------------------------

void entc_map_cursor_destroy (EntcMapCursor** pcursor)
{
  EntcMapCursor* cursor = *pcursor;
  
  free (cursor);
  *pcursor = NULL;
}

//-----------------------------------------------------------------------------

int entc_map_cursor_nextnode (EntcMapCursor* cursor, int dir)
{
  if (cursor->position < 0)
  {
    if (dir == cursor->direction)
    {
      if (cursor->node)
      {
        cursor->position = 0;
        return 1;
      }
    }
  }
  else
  {
    EntcMapNode n = cursor->node;
    
    if (n)
    {
      cursor->node = entc_map_node_next (n, dir);
      
      if (cursor->node)
      {
        cursor->position++;
        return 1;
      }
    }
  }
  
  return 0;
}

//-----------------------------------------------------------------------------

int entc_map_cursor_next (EntcMapCursor* cursor)
{
  return entc_map_cursor_nextnode (cursor, ENTC_MAP_RIGHT);
}

//-----------------------------------------------------------------------------

int entc_map_cursor_prev (EntcMapCursor* cursor)
{
  return entc_map_cursor_nextnode (cursor, ENTC_MAP_LEFT);
}

//-----------------------------------------------------------------------------

EntcMapNode entc_map_cursor_extract (EntcMap self, EntcMapCursor* cursor)
{
  EntcMapNode x = cursor->node;
  
  if (x)
  {
    if (entc_map_cursor_prev (cursor))
    {
      entc_map_extract (self, x);
    }
    else
    {
      x = entc_map_extract (self, x);
      
      if (cursor->direction == ENTC_DIRECTION_FORW)
      {
        cursor->node = entc_map_node_toTheLast (self->root, ENTC_MAP_LEFT);
      }
      else
      {
        cursor->node = entc_map_node_toTheLast (self->root, ENTC_MAP_RIGHT);
      }
      
      cursor->position = -1;
    }
    
    return x;
  }
  else
  {
    return NULL;
  }
}

//-----------------------------------------------------------------------------

void entc_map_cursor_erase (EntcMap self, EntcMapCursor* cursor)
{
  EntcMapNode node2 = entc_map_cursor_extract (self, cursor);
  
  entc_map_del_node (self, &node2);
}

//-----------------------------------------------------------------------------


