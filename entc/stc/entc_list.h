#ifndef __ENTC_STC__LIST__H
#define __ENTC_STC__LIST__H 1

#include "sys/entc_export.h"

//=============================================================================

struct EntcList_s; typedef struct EntcList_s* EntcList;
struct EntcListNode_s; typedef struct EntcListNode_s* EntcListNode;

typedef void (__STDCALL *fct_entc_list_onDestroy) (void* ptr);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcList          entc_list_new              (fct_entc_list_onDestroy);

__ENTC_LIBEX   void              entc_list_del              (EntcList*);

__ENTC_LIBEX   void              entc_list_clr              (EntcList);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcListNode      entc_list_push_back        (EntcList, void* data);

__ENTC_LIBEX   EntcListNode      entc_list_push_front       (EntcList, void* data);

__ENTC_LIBEX   void*             entc_list_pop_front        (EntcList);

__ENTC_LIBEX   void*             entc_list_pop_back         (EntcList);

__ENTC_LIBEX   unsigned long     entc_list_size             (EntcList);

__ENTC_LIBEX   int               entc_list_empty            (EntcList);

__ENTC_LIBEX   int               entc_list_hasContent       (EntcList);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void              entc_list_node_replace     (EntcList, EntcListNode, void* data);

__ENTC_LIBEX   void*             entc_list_node_data        (EntcListNode);

__ENTC_LIBEX   void*             entc_list_node_extract     (EntcList, EntcListNode);

__ENTC_LIBEX   void              entc_list_node_erase       (EntcList, EntcListNode);

__ENTC_LIBEX   EntcListNode      entc_list_node_next        (EntcListNode);

__ENTC_LIBEX   EntcListNode      entc_list_node_begin       (EntcList);

__ENTC_LIBEX   void              entc_list_node_swap        (EntcListNode, EntcListNode);

//-----------------------------------------------------------------------------

typedef int (__STDCALL *fct_entc_list_onCompare) (void* ptr1, void* ptr2);

__ENTC_LIBEX   void              entc_list_sort             (EntcList, fct_entc_list_onCompare);

//-----------------------------------------------------------------------------

typedef void* (__STDCALL *fct_entc_list_onClone) (void* ptr);

__ENTC_LIBEX   EntcList           entc_list_clone           (EntcList, fct_entc_list_onClone);

//-----------------------------------------------------------------------------

typedef struct
{
  
  EntcListNode node;
  
  int position;
  
  int direction;
  
} EntcListCursor;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcListCursor*   entc_list_cursor_create    (EntcList, int direction);

__ENTC_LIBEX   void              entc_list_cursor_destroy   (EntcListCursor**);

__ENTC_LIBEX   void              entc_list_cursor_init      (EntcList, EntcListCursor*, int direction);

__ENTC_LIBEX   int               entc_list_cursor_next      (EntcListCursor*);

__ENTC_LIBEX   int               entc_list_cursor_prev      (EntcListCursor*);

__ENTC_LIBEX   void              entc_list_cursor_erase     (EntcList, EntcListCursor*);

__ENTC_LIBEX   void*             entc_list_cursor_extract   (EntcList, EntcListCursor*);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcList          entc_list_slice_extract    (EntcList, EntcListNode nodeFrom, EntcListNode nodeTo);

__ENTC_LIBEX   void              entc_list_slice_insert     (EntcList, EntcListCursor*, EntcList* pslice);

//-----------------------------------------------------------------------------

#endif
