
#include "types/eclist.h"
#include "tests/ecenv.h"

#include <stdio.h>

//=============================================================================

static int __STDCALL test_stdlist_onDestroy (void* ptr)
{
  free (ptr);
  
  return 0;
}

//---------------------------------------------------------------------------

static void* __STDCALL test_stdlist_init (EcErr err)
{
  return eclist_create (test_stdlist_onDestroy);
}

//---------------------------------------------------------------------------

static void __STDCALL test_stdlist_done (void* ptr)
{
  EcList h = ptr;
  
  eclist_destroy (&h);
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  EcList h = ptr;
  int i;
  
  for (i = 0; i < 10; i++)
  {
    void* data = malloc(42);
    
    sprintf((char*)data, "hello world [%i]", i);
    
    eclist_push_back (h, data);
  }
  
  eclist_clear (h);
  
  // prepare compare values
  for (i = 0; i < 5; i++)
  {
    void* data = malloc(42);
    sprintf((char*)data, "hello world [%i]", i);
    
    eclist_push_back (h, data);
    
    // add for test to compare
    testctx_push_string (ctx, (const char*)data);
  }
  
  // forward
  {
    EcListCursor* cursor = eclist_cursor_create (h, 1);
    
    while (eclist_cursor_next (cursor))
    {
      void* data = eclist_data (cursor->node);
      
      // check ************
      testctx_assert (ctx, testctx_pop_tocomp (ctx, (const char*)data), "comp #1");
    }
    
    eclist_cursor_destroy (&cursor);
  }
  
  // backwards
  {
    EcListCursor* cursor = eclist_cursor_create (h, 0);
    
    while (eclist_cursor_prev (cursor))
    {
      void* data = eclist_data (cursor->node);
      
      printf ("data: %s\n", data);
    }
    
    eclist_cursor_destroy (&cursor);
  }
  
  // combine
  {
    EcListCursor* cursor = eclist_cursor_create (h, 1);
    void* data;    

    eclist_cursor_next (cursor);    // 0
    eclist_cursor_next (cursor);    // 1
    eclist_cursor_next (cursor);    // 2
    
    eclist_cursor_prev (cursor);    // 1
    
    data = eclist_data (cursor->node);
    
    printf ("comb: %s\n", data);
    
    eclist_cursor_destroy (&cursor);
  }
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcList h = ptr;
  int i;
  
  eclist_clear (h);
  
  // check ************
  testctx_assert (tctx, eclist_size (h) == 0, "check size #1");
  
  for (i = 0; i < 5; i++)
  {
    void* data = malloc(42);
    eclist_push_back (h, data);
  }
  
  // check ************
  testctx_assert (tctx, eclist_size (h) == 5, "check size #2");
  
  {
    EcListCursor cursor;
    
    eclist_cursor_init (h, &cursor, LIST_DIR_NEXT);
    
    eclist_erase (h, &cursor);
    
    // check ************
    testctx_assert (tctx, eclist_size (h) == 4, "check size #3");
    
    while (eclist_cursor_next (&cursor))
    {
      eclist_erase (h, &cursor);
    }
  }
  
  // check ************
  testctx_assert (tctx, eclist_size (h) == 0, "check size #4");
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlist_test3 (void* ptr, TestEnvContext tctx, EcErr err)
{
  EcList h = ptr;
  int i;
  EcListNode fpos;
  EcListNode lpos;
  
  eclist_clear (h);
  
  // check ************
  testctx_assert (tctx, eclist_size (h) == 0, "check size #1");
  
  for (i = 0; i < 10; i++)
  {
    void* data = malloc(42);
    sprintf((char*)data, "cloud [%i]", i);
    
    eclist_push_back (h, data);
  }
    
  {
    EcList slice;
    EcListCursor cursor;
    
    eclist_cursor_init (h, &cursor, LIST_DIR_NEXT);
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #0");
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #1");
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #2");
    
    fpos = cursor.node;
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #3");
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #4");
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #5");
    
    lpos = cursor.node;
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #6");
    
    testctx_assert (tctx, eclist_cursor_next (&cursor), "cursor pos #7");
    
    slice = eclist_slice (h, fpos, lpos);
    
    testctx_assert (tctx, eclist_size (slice) == 4, "check size slice");
    testctx_assert (tctx, eclist_size (h) == 6, "check rest slice");
    
    eclist_insert_slice (h, &cursor, &slice);
    
    testctx_assert (tctx, slice == NULL, "del slice");
    
    testctx_assert (tctx, eclist_size (h) == 10, "check size slice #2");
  }
  
  {
    EcListCursor cursor;
    
    eclist_cursor_init (h, &cursor, LIST_DIR_NEXT);
    
    while (eclist_cursor_next (&cursor))
    {
      void* data = eclist_data(cursor.node);
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
  EcList h = (EcList)ptr;
  
  eclist_clear (h);
  
  eclist_push_back (h, ecstr_copy ("Sort 7"));
  eclist_push_back (h, ecstr_copy ("Sort 4"));
  eclist_push_back (h, ecstr_copy ("Sort 9"));
  eclist_push_back (h, ecstr_copy ("Sort 3"));
  eclist_push_back (h, ecstr_copy ("Sort 1"));
  eclist_push_back (h, ecstr_copy ("Sort 6"));
  eclist_push_back (h, ecstr_copy ("Sort 8"));
  eclist_push_back (h, ecstr_copy ("Sort 2"));
  eclist_push_back (h, ecstr_copy ("Sort 5"));
  
  eclist_sort (h, test_stdlist_test4_onCompare);
  
  {
    EcListCursor* cursor = eclist_cursor_create (h, 1);
    
    while (eclist_cursor_next (cursor))
    {
      void* data = eclist_data (cursor->node);
      
      printf ("data: %s\n", data);
    }
    
    eclist_cursor_destroy (&cursor);
  }
  
  // backwards
  {
    EcListCursor* cursor = eclist_cursor_create (h, 0);
    
    while (eclist_cursor_prev (cursor))
    {
      void* data = eclist_data (cursor->node);
      
      printf ("data: %s\n", data);
    }
    
    eclist_cursor_destroy (&cursor);
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
