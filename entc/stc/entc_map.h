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

#ifndef __ENTC_STC__MAP__H
#define __ENTC_STC__MAP__H 1

#include "sys/entc_export.h"

//=============================================================================

/* this class implements a balanced binary tree
 *
 * -> there is a simple implementation of a binary tree used as the basis tree
 * -> the balancing part is implemented by rotating of the tree nodes 
 *    following the RED / BLACK balanced tree principles
 * -> the cursor part is not a basic feature of balanced tree and was
 *    implemented following the entc_list API for cursor's
 * 
 * remarks: maybe in the future the BST and RB tree can be split into separated classes
 *          the rotating algorithm is complex and was not fully tested in cases of removing nodes
 * 
 * -> the result might be: that the tree is not optimal balanced
 */

//=============================================================================

struct EntcMap_s; typedef struct EntcMap_s* EntcMap;
struct EntcMapNode_s; typedef struct EntcMapNode_s* EntcMapNode;

typedef int   (__STDCALL *fct_entc_map_cmp)      (const void* a, const void* b);
typedef void  (__STDCALL *fct_entc_map_destroy)  (void* key, void* val);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcMap           entc_map_new               (fct_entc_map_cmp, fct_entc_map_destroy);

__ENTC_LIBEX   void              entc_map_del               (EntcMap*);

__ENTC_LIBEX   void              entc_map_clr               (EntcMap);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcMapNode       entc_map_insert            (EntcMap, void* key, void* data);

__ENTC_LIBEX   EntcMapNode       entc_map_find              (EntcMap, void* key);

__ENTC_LIBEX   void              entc_map_erase             (EntcMap, EntcMapNode);       // removes the node, calls the onDestroy callback and releases the node

__ENTC_LIBEX   EntcMapNode       entc_map_extract           (EntcMap, EntcMapNode);       // extracts the node from the container and returns it

__ENTC_LIBEX   void              entc_map_del_node          (EntcMap, EntcMapNode*);      // calls the onDestroy callback and releases the node

__ENTC_LIBEX   unsigned long     entc_map_size              (EntcMap);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void*             entc_map_node_value        (EntcMapNode);

__ENTC_LIBEX   void*             entc_map_node_key          (EntcMapNode);

__ENTC_LIBEX   void              entc_map_node_del          (EntcMapNode*);               // don't calls the onDestroy method, only releases the memory

//-----------------------------------------------------------------------------

typedef void* (__STDCALL *fct_entc_map_onClone) (void* ptr);

__ENTC_LIBEX   EntcMap           entc_map_clone             (EntcMap, fct_entc_map_onClone onCloneKey, fct_entc_map_onClone onCloneVal);

//-----------------------------------------------------------------------------

typedef struct
{  
  EntcMapNode node;    // the tree node
  int direction;       // the direction of the cursor
  int position;        // the current position
  
} EntcMapCursor;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcMapCursor*    entc_map_cursor_new        (EntcMap, int direction);

__ENTC_LIBEX   void              entc_map_cursor_del        (EntcMapCursor**);

__ENTC_LIBEX   void              entc_map_cursor_init       (EntcMap, EntcMapCursor*, int direction);

__ENTC_LIBEX   int               entc_map_cursor_next       (EntcMapCursor*);

__ENTC_LIBEX   int               entc_map_cursor_prev       (EntcMapCursor*);

__ENTC_LIBEX   void              entc_map_cursor_erase      (EntcMap, EntcMapCursor*);

__ENTC_LIBEX   EntcMapNode       entc_map_cursor_extract    (EntcMap, EntcMapCursor*);

//-----------------------------------------------------------------------------

#endif
