#include "ecmap.h"

#include "system/macros.h"

//=============================================================================

#define ENTC_ECMAP_RED     1
#define ENTC_ECMAP_BLACK   0

#define ENTC_ECMAP_LEFT    0
#define ENTC_ECMAP_RIGHT   1

static const int ENTC_ECMAP_INV[2] = {ENTC_ECMAP_RIGHT, ENTC_ECMAP_LEFT};

//-----------------------------------------------------------------------------

typedef struct EcMapNode_s* EcMapNode;
struct EcMapNode_s
{
   int color;

   EcMapNode parent;
   EcMapNode link[2];

   // content
   void* val;
   void* key;
};

//-----------------------------------------------------------------------------

EcMapNode ecmap_node_create (EcMapNode parent, void* key, void* val)
{
   EcMapNode self = ENTC_NEW(struct EcMapNode_s);

   self->color = ENTC_ECMAP_RED;

   self->parent = parent;
   self->link[ENTC_ECMAP_LEFT] = NULL;
   self->link[ENTC_ECMAP_RIGHT] = NULL;

   self->val = val;
   self->key = key;

   return self;
}

//-----------------------------------------------------------------------------

void ecmap_node_destroy (EcMapNode* pself)
{
  ENTC_DEL(pself, struct EcMapNode_s);
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

EcMapNode ecmap_node_left (EcMapNode n)
{
   return n == NULL ? NULL : n->link[ENTC_ECMAP_LEFT];
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_node_right (EcMapNode n)
{
   return n == NULL ? NULL : n->link[ENTC_ECMAP_RIGHT];
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_node_toTheLast (EcMapNode n, int dir)
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

EcMapNode ecmap_node_next (EcMapNode n, int dir)
{
   EcMapNode m = n->link[dir];
   EcMapNode p = n->parent;

   if (m)
   {
      return ecmap_node_toTheLast (m, ENTC_ECMAP_INV[dir]);
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

EcMapNode ecmap_siblingOf (EcMapNode n)
{
   return (n == NULL || n->parent == NULL) ? NULL : (n == n->parent->link[ENTC_ECMAP_LEFT] ? n->parent->link[ENTC_ECMAP_RIGHT] : n->parent->link[ENTC_ECMAP_LEFT]);
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_grandparentOf (EcMapNode n)
{
   return (n == NULL || n->parent == NULL) ? NULL : n->parent->parent;
}

//-----------------------------------------------------------------------------

int ecmap_isRed (EcMapNode n)
{
   return n && n->color == ENTC_ECMAP_RED;
}

//-----------------------------------------------------------------------------

int ecmap_isBlack (EcMapNode n)
{
   return n && n->color == ENTC_ECMAP_BLACK;
}

//-----------------------------------------------------------------------------

void ecmap_setColor (EcMapNode n, int color)
{
   if (n)
   {
      n->color = color;
   }
}

//=============================================================================

struct EcMap_s
{
   EcMapNode root;

   fct_ecmap_cmp onCompare;

   fct_ecmap_destroy onDestroy;

   size_t size;
};

//-----------------------------------------------------------------------------

static int __STDCALL ecmap_node_cmp (const void* a, const void* b)
{
   const char* s1 = a;
   const char* s2 = b;

   return strcmp(s1, s2);
}

//-----------------------------------------------------------------------------

void ecmap_rotate (EcMap self, EcMapNode x, int dir)
{
   EcMapNode y = x->link[ENTC_ECMAP_INV[dir]];
   if (y)
   {
      EcMapNode h = y->link[dir];
      EcMapNode p = x->parent;

      // turn y's left subtree into x's right subtree
      x->link[ENTC_ECMAP_INV[dir]] = h;
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
         if (p->link[ENTC_ECMAP_LEFT] == x)
         {
            p->link[ENTC_ECMAP_LEFT] = y;
         }
         else
         {
            p->link[ENTC_ECMAP_RIGHT] = y;
         }
      }
      else  // x was root node
      {
         self->root = y;
      }
   }
}

//-----------------------------------------------------------------------------

EcMap ecmap_create (fct_ecmap_cmp onCompare, fct_ecmap_destroy onDestroy)
{
   EcMap self = ENTC_NEW(struct EcMap_s);

   self->root = NULL;
   self->size = 0;

   self->onCompare = onCompare ? onCompare : ecmap_node_cmp;
   self->onDestroy = onDestroy;

   return self;
}

//-----------------------------------------------------------------------------

void ecmap_destroy_node (EcMap self, EcMapNode* pself)
{
   EcMapNode n = *pself;

   if (self->onDestroy)
   {
      self->onDestroy (n->key, n->val);
   }

   ecmap_node_destroy (pself);
}

//-----------------------------------------------------------------------------

void ecmap_clear (EcMap self)
{
   EcMapNode n = self->root;

   while (n)
   {
      EcMapNode p;

      if (n->link[ENTC_ECMAP_LEFT])
      {
         p = n->link[ENTC_ECMAP_LEFT];

         // mark that we already went this way
         n->link[ENTC_ECMAP_LEFT] = NULL;

         n = p;

         continue;
      }

      if (n->link[ENTC_ECMAP_RIGHT])
      {
         p = n->link[ENTC_ECMAP_RIGHT];

         // mark that we already went this way
         n->link[ENTC_ECMAP_RIGHT] = NULL;

         n = p;

         continue;
      }

      // no links available -> delete
      p = n->parent;

      ecmap_destroy_node (self, &n);

      n = p;
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
   EcMapNode n = self->root;

   while (n)
   {
      int res = self->onCompare (n->key, key);

      if (res < 0)
      {
         n = n->link[ENTC_ECMAP_RIGHT];
      }
      else if (res > 0)
      {
         n = n->link[ENTC_ECMAP_LEFT];
      }
      else
      {
         return n;
      }
   }

   return NULL;
}

//-----------------------------------------------------------------------------

void ecmap_node_swap (EcMapNode n1, EcMapNode n2)
{
   void* key = n1->key;
   void* val = n1->val;

   n1->key = n2->key;
   n1->val = n2->val;

   n2->key = key;
   n2->val = val;
}

//-----------------------------------------------------------------------------

EcMapNode stdbst_insert (EcMap self, EcMapNode z)
{
   EcMapNode y = NULL;
   EcMapNode x = self->root;

   while (x)
   {
      int res;

      y = x;

      res = self->onCompare (z->key, x->key);
      if (res < 0)
      {
         x = x->link[ENTC_ECMAP_LEFT];
      }
      else if (res > 0)
      {
         x = x->link[ENTC_ECMAP_RIGHT];
      }
      else
      {
         ecmap_node_swap (x, z);
         return x;
      }
   }

   z->parent = y;

   if (y)
   {
      int res = self->onCompare (z->key, y->key);
      if (res < 0)
      {
         y->link[ENTC_ECMAP_LEFT] = z;
      }
      else if (res > 0)
      {
         y->link[ENTC_ECMAP_RIGHT] = z;
      }
      else
      {
         ecmap_node_swap (y, z);
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

void ecmap_insert_balance (EcMap self, EcMapNode x)
{
   EcMapNode n = x;
   n->color = ENTC_ECMAP_RED;

   while (n && ecmap_isRed (n->parent))
   {
      EcMapNode p = n->parent;   // p is always valid
      EcMapNode s = ecmap_siblingOf (p);

      if (ecmap_isRed (s))
      {
         p->color = ENTC_ECMAP_BLACK;
         s->color = ENTC_ECMAP_BLACK;

         s->parent->color = ENTC_ECMAP_RED;
         n = s->parent;
      }
      else
      {
         EcMapNode g = p->parent;
         if (g)
         {
            if (p == g->link[ENTC_ECMAP_LEFT])
            {
               if (n == p->link[ENTC_ECMAP_RIGHT])
               {
                  n = p;
                  ecmap_rotate (self, n, ENTC_ECMAP_LEFT);
               }

               // reset
               p = n->parent;   // p is always valid
               g = p->parent;   // might be NULL

               p->color = ENTC_ECMAP_BLACK;

               if (g)
               {
                  g->color = ENTC_ECMAP_RED;
                  ecmap_rotate (self, g, ENTC_ECMAP_RIGHT);
               }
            }
            else
            {
               if (n == p->link[ENTC_ECMAP_LEFT])
               {
                  n = p;
                  ecmap_rotate (self, n, ENTC_ECMAP_RIGHT);
               }

               // reset
               p = n->parent;   // p is always valid
               g = p->parent;   // might be NULL

               p->color = ENTC_ECMAP_BLACK;

               if (g)
               {
                  g->color = ENTC_ECMAP_RED;
                  ecmap_rotate (self, g, ENTC_ECMAP_LEFT);
               }
            }
         }
         else
         {
            p->color = ENTC_ECMAP_BLACK;
         }
      }
   }

   self->root->color = ENTC_ECMAP_BLACK;
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_insert (EcMap self, void* key, void* val)
{
   EcMapNode n = ecmap_node_create (NULL, key, val);

   if (stdbst_insert (self, n) == n)
   {
      // re-balance the tree
      ecmap_insert_balance (self, n);
   }
   else
   {
      ecmap_destroy_node (self, &n);
   }

   return n;
}

//-----------------------------------------------------------------------------

int ecmap_maxdepth (EcMapNode n)
{
   int depth = 0;

   if (n)
   {
      int dp = 1;
      int dl = 0;
      int dr = 0;

      if (n->link[ENTC_ECMAP_LEFT])
      {
         dl = ecmap_maxdepth (n->link[ENTC_ECMAP_LEFT]);
      }

      if (n->link[ENTC_ECMAP_RIGHT])
      {
         dr = ecmap_maxdepth (n->link[ENTC_ECMAP_LEFT]);
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

int ecmap_maxtreedepth (EcMap self)
{
   return ecmap_maxdepth (self->root);
}

//-----------------------------------------------------------------------------

void ecmap_tree_remove_balance (EcMap self, EcMapNode x)
{
   EcMapNode p = x->parent;
   EcMapNode s;
   int d;

   if (p == NULL)
   {
      // Case 1 - node is the new root.

      return;
   }

   // get sibling
   if (p->link[ENTC_ECMAP_LEFT] == x)
   {
      d = ENTC_ECMAP_RIGHT;
   }
   else
   {
      d = ENTC_ECMAP_LEFT;
   }

   s = p->link[d];

   if (s)
   {
      if (s->color == ENTC_ECMAP_RED)
      {
         // Case 2 - sibling is red.

         p->color = ENTC_ECMAP_RED;
         s->color = ENTC_ECMAP_BLACK;

         ecmap_rotate (self, p, ENTC_ECMAP_INV[d]);

         // Rotation, need to update parent/sibling
         //ecmap_rotate (self, p, d);

         p = x->parent;

         if (p)
         {
            // get sibling
            if (p->link[ENTC_ECMAP_LEFT] == x)
            {
               d = ENTC_ECMAP_RIGHT;
            }
            else
            {
               d = ENTC_ECMAP_LEFT;
            }

            s = p->link[d];
         }
         else
         {
            s = NULL;
         }
      }

      if (s && (s->color == ENTC_ECMAP_BLACK))
      {
         if (ecmap_isBlack (s->link[ENTC_ECMAP_RIGHT]) && ecmap_isBlack (s->link[ENTC_ECMAP_LEFT]))
         {
            if (p->color == ENTC_ECMAP_BLACK)
            {
               // Case 3 - parent, sibling, and sibling's children are black.
               s->color = ENTC_ECMAP_RED;

               ecmap_tree_remove_balance (self, p);

               return;
            }
            else
            {
               // Case 4 - sibling and sibling's children are black, but parent is red.
               s->color = ENTC_ECMAP_RED;
               p->color = ENTC_ECMAP_BLACK;

               return;
            }
         }
         else
         {
            // Case 5 - sibling is black, sibling's left child is red,
            // sibling's right child is black, and node is the left child of
            // its parent.
            if ((x == p->link[ENTC_ECMAP_LEFT]) && ecmap_isRed (s->link[ENTC_ECMAP_LEFT]) && ecmap_isBlack (s->link[ENTC_ECMAP_RIGHT]))
            {
               s->color = ENTC_ECMAP_RED;

               ecmap_rotate (self, p, ENTC_ECMAP_RIGHT);

               p = x->parent;

               if (p)
               {
                  // get sibling
                  if (p->link[ENTC_ECMAP_LEFT] == x)
                  {
                     d = ENTC_ECMAP_RIGHT;
                  }
                  else
                  {
                     d = ENTC_ECMAP_LEFT;
                  }

                  s = p->link[d];
               }
               else
               {
                  s = NULL;
               }
            }
            else if ((x == p->link[ENTC_ECMAP_RIGHT]) && ecmap_isBlack (s->link[ENTC_ECMAP_LEFT]) && ecmap_isRed (s->link[ENTC_ECMAP_RIGHT]))
            {
               s->color = ENTC_ECMAP_RED;

               ecmap_rotate (self, p, ENTC_ECMAP_LEFT);

               p = x->parent;

               if (p)
               {
                  // get sibling
                  if (p->link[ENTC_ECMAP_LEFT] == x)
                  {
                     d = ENTC_ECMAP_RIGHT;
                  }
                  else
                  {
                     d = ENTC_ECMAP_LEFT;
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
         p->color = ENTC_ECMAP_BLACK;

         if (x == p->link[ENTC_ECMAP_LEFT])
         {
            ecmap_setColor (s->link[ENTC_ECMAP_RIGHT], ENTC_ECMAP_BLACK);

            ecmap_rotate (self, p, ENTC_ECMAP_LEFT);
         }
         else
         {
            ecmap_setColor (s->link[ENTC_ECMAP_LEFT], ENTC_ECMAP_BLACK);

            ecmap_rotate (self, p, ENTC_ECMAP_RIGHT);
         }
      }

   }

   /*


   while (p && x->color == ENTC_ECMAP_BLACK)
   {
      int d;
      EcMapNode s;

      if (p->link[ENTC_ECMAP_LEFT] == x)
      {
         d = ENTC_ECMAP_LEFT;
      }
      else
      {
         d = ENTC_ECMAP_RIGHT;
      }

      s = p->link[ENTC_ECMAP_INV[d]];

      if (s)
      {
         if (s->color == ENTC_ECMAP_RED)
         {
            s->color = ENTC_ECMAP_BLACK;
            p->color = ENTC_ECMAP_RED;

            ecmap_rotate (self, p, d);

            p = x->parent;
            s = p->link[ENTC_ECMAP_INV[d]];
         }

         if ((s->link[ENTC_ECMAP_LEFT]->color == ENTC_ECMAP_BLACK) && (s->link[ENTC_ECMAP_RIGHT]->color == ENTC_ECMAP_BLACK))
         {
            s->color = ENTC_ECMAP_RED;
            x = x->parent;
         }
         else
         {
            if (s->link[ENTC_ECMAP_INV[d]]->color == ENTC_ECMAP_BLACK)
            {
               s->link[ENTC_ECMAP_LEFT]->color = ENTC_ECMAP_BLACK;
               s->color = ENTC_ECMAP_RED;

               ecmap_rotate (self, s, ENTC_ECMAP_INV[d]);

               s = x->parent->link[ENTC_ECMAP_INV[d]];
            }

            s->color = x->parent->color;
            x->parent->color = ENTC_ECMAP_BLACK;
            s->link[ENTC_ECMAP_INV[d]]->color = ENTC_ECMAP_BLACK;

            ecmap_rotate (self, x->parent, d);
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

void stdbst_replace (EcMapNode a, EcMapNode b)
{
   if (a != b)
   {
      int color;

      // swap content of nodes
      ecmap_node_swap (a, b);

      // swap the color
      color = a->color;
      a->color = b->color;
      b->color = color;
   }
}

//-----------------------------------------------------------------------------

EcMapNode stdbst_delete (EcMap self, EcMapNode x)
{
   EcMapNode n = x;
   EcMapNode p;
   EcMapNode c = NULL;
   EcMapNode l = n->link[ENTC_ECMAP_LEFT];
   EcMapNode r = n->link[ENTC_ECMAP_RIGHT];

   while (l && r)
   {
      if (r)
      {
         n = ecmap_node_next (n, ENTC_ECMAP_RIGHT);  // successor of n
      }
      else
      {
         n = ecmap_node_next (n, ENTC_ECMAP_LEFT);  // predecessor of n
      }

      l = n->link[ENTC_ECMAP_LEFT];
      r = n->link[ENTC_ECMAP_RIGHT];
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

      n->link[ENTC_ECMAP_LEFT] = c->link[ENTC_ECMAP_LEFT];

      if (n->link[ENTC_ECMAP_LEFT])
      {
         n->link[ENTC_ECMAP_LEFT]->parent = n;
      }

      n->link[ENTC_ECMAP_RIGHT] = c->link[ENTC_ECMAP_RIGHT];

      if (n->link[ENTC_ECMAP_RIGHT])
      {
         n->link[ENTC_ECMAP_RIGHT]->parent = n;
      }

      if (n->color == ENTC_ECMAP_BLACK)
      {
         if (c->color == ENTC_ECMAP_RED)
         {
            c->color = ENTC_ECMAP_BLACK;
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
         if (p->link[ENTC_ECMAP_LEFT] == n)
         {
            p->link[ENTC_ECMAP_LEFT] = NULL;
         }
         else
         {
            p->link[ENTC_ECMAP_RIGHT] = NULL;
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

EcMapNode ecmap_extract (EcMap self, EcMapNode node)
{
   if (node)
   {
      EcMapNode x = stdbst_delete (self, node);

      /*
      if (node->color == ENTC_ECMAP_BLACK)
      {
         // re-balance the tree
         ecmap_tree_remove_balance (self, x);
      }
      */

      return x;
   }

   return node;
}

//-----------------------------------------------------------------------------

void ecmap_erase (EcMap self, EcMapNode node)
{
   if (node)
   {
      EcMapNode node2 = ecmap_extract (self, node);

      ecmap_destroy_node (self, &node2);
   }
}

//-----------------------------------------------------------------------------

unsigned long ecmap_size (EcMap self)
{
   return self->size;
}

//-----------------------------------------------------------------------------

int ecmap_validate_parent (EcMapNode n, EcMapNode parent)
{
   if (n == NULL)
   {
      return 1;
   }

   if (n->parent != parent)
   {
      return 0;
   }

   if (ecmap_validate_parent (n->link[ENTC_ECMAP_LEFT], n) == 0)
   {
      return 0;
   }

   if (ecmap_validate_parent (n->link[ENTC_ECMAP_RIGHT], n) == 0)
   {
      return 0;
   }

   return 1;
}

//-----------------------------------------------------------------------------

int ecmap_validate (EcMap self)
{
   return ecmap_validate_parent (self->root, NULL);
}

//=============================================================================

void ecmap_cursor_init (EcMap self, EcMapCursor* cursor, int direction)
{
   if (direction == ENTC_DIRECTION_FORW)
   {
      cursor->node = ecmap_node_toTheLast (self->root, ENTC_ECMAP_LEFT);
   }
   else
   {
      cursor->node = ecmap_node_toTheLast (self->root, ENTC_ECMAP_RIGHT);
   }

   cursor->position = -1;
   cursor->direction = direction;
}

//-----------------------------------------------------------------------------

EcMapCursor* ecmap_cursor_create (EcMap self, int direction)
{
   EcMapCursor* cursor = (EcMapCursor*)malloc (sizeof(EcMapCursor));

   ecmap_cursor_init (self, cursor, direction);

   return cursor;
}

//-----------------------------------------------------------------------------

void ecmap_cursor_destroy (EcMapCursor** pcursor)
{
   EcMapCursor* cursor = *pcursor;

   free (cursor);
   *pcursor = NULL;
}

//-----------------------------------------------------------------------------

int ecmap_cursor_nextnode (EcMapCursor* cursor, int dir)
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
      EcMapNode n = cursor->node;

      if (n)
      {
         cursor->node = ecmap_node_next (n, dir);

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

int ecmap_cursor_next (EcMapCursor* cursor)
{
   return ecmap_cursor_nextnode (cursor, ENTC_ECMAP_RIGHT);
}

//-----------------------------------------------------------------------------

int ecmap_cursor_prev (EcMapCursor* cursor)
{
   return ecmap_cursor_nextnode (cursor, ENTC_ECMAP_LEFT);
}

//-----------------------------------------------------------------------------

EcMapNode ecmap_cursor_extract (EcMap self, EcMapCursor* cursor)
{
   EcMapNode x = cursor->node;

   if (x)
   {
      if (ecmap_cursor_prev (cursor))
      {
         ecmap_extract (self, x);
      }
      else
      {
         x = ecmap_extract (self, x);

         if (cursor->direction == ENTC_DIRECTION_FORW)
         {
            cursor->node = ecmap_node_toTheLast (self->root, ENTC_ECMAP_LEFT);
         }
         else
         {
            cursor->node = ecmap_node_toTheLast (self->root, ENTC_ECMAP_RIGHT);
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

void ecmap_cursor_erase (EcMap self, EcMapCursor* cursor)
{
   EcMapNode node2 = ecmap_cursor_extract (self, cursor);

   ecmap_destroy_node (self, &node2);
}

//-----------------------------------------------------------------------------

