#include "stc/entc_list.h"
#include "tests/ecenv.h"

#include <stdio.h>
#include <string.h>

//=============================================================================

static void __STDCALL test_stdlist_onDestroy (void* ptr)
{
  free (ptr);
}

//---------------------------------------------------------------------------

static void* __STDCALL test_stdlist_init (EcErr err)
{
  return entc_list_new (test_stdlist_onDestroy);
}

//---------------------------------------------------------------------------

static void __STDCALL test_stdlist_done (void* ptr)
{
  EntcList h = ptr;
  
  entc_list_del (&h);
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EntcList h = ptr;
  int i;
  
  for (i = 0; i < 10; i++)
  {
    void* data = ecstr_create_fmt (42, "hello world [%i]", i);   

    entc_list_push_back (h, data);
  }
  
  entc_list_clr (h);
  
  // prepare compare values
  for (i = 0; i < 5; i++)
  {
    void* data = ecstr_create_fmt (42, "hello world [%i]", i);    
    
    entc_list_push_back (h, data);
    
    // add for test to compare
    testctx_push_string (ctx, (const char*)data);
  }
  
  // forward
  {
    EntcListCursor* cursor = entc_list_cursor_new (h, 1);
    
    while (entc_list_cursor_next (cursor))
    {
      void* data = entc_list_node_data (cursor->node);
      
      // check ************
      testctx_assert (ctx, testctx_pop_tocomp (ctx, (const char*)data), "comp #1");
    }
    
    entc_list_cursor_del (&cursor);
  }
  
  // backwards
  {
    EntcListCursor* cursor = entc_list_cursor_new (h, 0);
    
    while (entc_list_cursor_prev (cursor))
    {
      char* data = entc_list_node_data (cursor->node);
      
      printf ("data: %s\n", data);
    }
    
    entc_list_cursor_del (&cursor);
  }
  
  // combine
  {
    EntcListCursor* cursor = entc_list_cursor_new (h, 1);
    char* data;    

    entc_list_cursor_next (cursor);    // 0
    entc_list_cursor_next (cursor);    // 1
    entc_list_cursor_next (cursor);    // 2
    
    entc_list_cursor_prev (cursor);    // 1
    
    data = entc_list_node_data (cursor->node);
    
    printf ("comb: %s\n", data);
    
    entc_list_cursor_del (&cursor);
  }
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EntcList h = ptr;
  int i;
  
  entc_list_clr (h);
  
  // check ************
  testctx_assert (tctx, entc_list_size (h) == 0, "check size #1");
  
  for (i = 0; i < 5; i++)
  {
    void* data = malloc(42);
    entc_list_push_back (h, data);
  }
  
  // check ************
  testctx_assert (tctx, entc_list_size (h) == 5, "check size #2");
  
  {
    EntcListCursor cursor;
    
    entc_list_cursor_init (h, &cursor, ENTC_DIRECTION_FORW);
    
    entc_list_cursor_erase (h, &cursor);
    
    // check ************
    testctx_assert (tctx, entc_list_size (h) == 4, "check size #3");
    
    while (entc_list_cursor_next (&cursor))
    {
      entc_list_cursor_erase (h, &cursor);
    }
  }
  
  // check ************
  testctx_assert (tctx, entc_list_size (h) == 0, "check size #4");
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test3 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EntcList h = ptr;
  int i;
  EntcListNode fpos;
  EntcListNode lpos;
  
  entc_list_clr (h);
  
  // check ************
  testctx_assert (tctx, entc_list_size (h) == 0, "check size #1");
  
  for (i = 0; i < 10; i++)
  {
    void* data = ecstr_create_fmt (42, "cloud [%i]", i);    
    
    entc_list_push_back (h, data);
  }
    
  {
    EntcList slice;
    EntcListCursor cursor;
    
    entc_list_cursor_init (h, &cursor, ENTC_DIRECTION_FORW);
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #0");
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #1");
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #2");
    
    fpos = cursor.node;
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #3");
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #4");
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #5");
    
    lpos = cursor.node;
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #6");
    
    testctx_assert (tctx, entc_list_cursor_next (&cursor), "cursor pos #7");
    
    slice = entc_list_slice_extract (h, fpos, lpos);
    
    testctx_assert (tctx, entc_list_size (slice) == 4, "check size slice");
    testctx_assert (tctx, entc_list_size (h) == 6, "check rest slice");
    
    entc_list_slice_insert (h, &cursor, &slice);
    
    testctx_assert (tctx, slice == NULL, "del slice");
    
    testctx_assert (tctx, entc_list_size (h) == 10, "check size slice #2");
  }
  
  {
    EntcListCursor cursor;
    
    entc_list_cursor_init (h, &cursor, ENTC_DIRECTION_FORW);
    
    while (entc_list_cursor_next (&cursor))
    {
      char* data = entc_list_node_data(cursor.node);
      printf ("data: %s\n", data);
    }
  }
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test4_onCompare (void* ptr1, void* ptr2)
{
  return strcmp(ptr1, ptr2);
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test4 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EntcList h = (EntcList)ptr;
  
  entc_list_clr (h);
  
  entc_list_push_back (h, ecstr_copy ("Sort 7"));
  entc_list_push_back (h, ecstr_copy ("Sort 4"));
  entc_list_push_back (h, ecstr_copy ("Sort 9"));
  entc_list_push_back (h, ecstr_copy ("Sort 3"));
  entc_list_push_back (h, ecstr_copy ("Sort 1"));
  entc_list_push_back (h, ecstr_copy ("Sort 6"));
  entc_list_push_back (h, ecstr_copy ("Sort 8"));
  entc_list_push_back (h, ecstr_copy ("Sort 2"));
  entc_list_push_back (h, ecstr_copy ("Sort 5"));
  
  entc_list_sort (h, test_stdlist_test4_onCompare);
  
  {
    EntcListCursor* cursor = entc_list_cursor_new (h, 1);
    
    while (entc_list_cursor_next (cursor))
    {
      char* data = entc_list_node_data (cursor->node);
      
      printf ("data: %s\n", data);
    }
    
    entc_list_cursor_del (&cursor);
  }
  
  // backwards
  {
    EntcListCursor* cursor = entc_list_cursor_new (h, 0);
    
    while (entc_list_cursor_prev (cursor))
    {
      char* data = entc_list_node_data (cursor->node);
      
      printf ("data: %s\n", data);
    }
    
    entc_list_cursor_del (&cursor);
  }
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "List Test1", test_stdlist_init, test_stdlist_done, test_stdlist_test1);
  testenv_reg (te, "List Test2", test_stdlist_init, test_stdlist_done, test_stdlist_test2);
  testenv_reg (te, "List Test3", test_stdlist_init, test_stdlist_done, test_stdlist_test3);
  testenv_reg (te, "List Test4", test_stdlist_init, test_stdlist_done, test_stdlist_test4);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
