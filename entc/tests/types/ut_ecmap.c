
#include "types/ecmap.h"
#include "tests/ecenv.h"

#include <stdio.h>
#include <memory.h>

#include "system/macros.h"

//=============================================================================

static void __STDCALL test_stdlist_onItemDestroy (void* key, void* val)
{
  {
    free (key);
  }
  {
    free (val);
  }
}

//---------------------------------------------------------------------------

static void* __STDCALL test_ecmap_init (EcErr err)
{
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecmap_done (void* ptr)
{

}

//---------------------------------------------------------------------------

static int __STDCALL test_ecmap_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  int i;
  EcMap map01 = ecmap_create (NULL, test_stdlist_onItemDestroy);
  
  for (i = 0; i < 10; i++)
  {
    void* key = ecstr_create_fmt (42, "item [%i]", i);
    void* val = ecstr_create_fmt (42, "value [%i]", i);

	ecmap_insert(map01, key, val);
  }
  
  ecmap_clear (map01);
  
  for (i = 0; i < 10; i++)
  {
    void* key = ecstr_create_fmt (42, "item [%i]", i);
    void* val = ecstr_create_fmt (42, "value [%i]", i);

    ecmap_insert(map01, key, val);
  }

  // forward
  {
    EcMapCursor* cursor = ecmap_cursor_create (map01, LIST_DIR_NEXT);
    
    while (ecmap_cursor_next (cursor))
    {
      void* key = ecmap_node_key (cursor->node);
      void* val = ecmap_node_value (cursor->node);

      printf ("map item '%s':'%s'\n", key, val);
    }
    
    ecmap_cursor_destroy (&cursor);
  }

  // backwards
  {
    EcMapCursor* cursor = ecmap_cursor_create (map01, LIST_DIR_PREV);
    
    while (ecmap_cursor_prev (cursor))
    {
      void* key = ecmap_node_key (cursor->node);
      void* val = ecmap_node_value (cursor->node);
      
      printf ("map item '%s':'%s'\n", key, val);
    }
    
    ecmap_cursor_destroy (&cursor);
  }

  // combine
  {
    EcMapCursor* cursor = ecmap_cursor_create (map01, LIST_DIR_NEXT);
    
    ecmap_cursor_next (cursor);    // 0
    ecmap_cursor_next (cursor);    // 1
    ecmap_cursor_next (cursor);    // 2
    
    ecmap_cursor_prev (cursor);    // 1
    
    {
      void* key = ecmap_node_key (cursor->node);
      void* val = ecmap_node_value (cursor->node);
      
      printf ("map item '%s':'%s'\n", key, val);
    }
    
    ecmap_cursor_destroy (&cursor);
  }
  
  ecmap_destroy (&map01);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecmap_test2 (void* ptr, TestEnvContext ctx, EcErr err)
{
  int i;
  EcMap map01 = ecmap_create (NULL, test_stdlist_onItemDestroy);
  
  for (i = 0; i < 10; i++)
  {
    void* key = ecstr_create_fmt (42, "item [%i]", i);
    void* val = ecstr_create_fmt (42, "value [%i]", i);
    
    ecmap_insert(map01, key, val);
  }
 
  {
    EcMapCursor cursor;
    
    ecmap_cursor_init (map01, &cursor, LIST_DIR_NEXT);
    
    ecmap_cursor_erase (map01, &cursor);

    {
      void* key = ecmap_node_key (cursor.node);
      void* val = ecmap_node_value (cursor.node);
      
      printf ("cur [%lu] item '%s':'%s'\n", ecmap_size (map01), key, val);
    }
   
    while (ecmap_cursor_next (&cursor))
    {
      ecmap_cursor_erase (map01, &cursor);
    }
    
    printf ("cur [%lu]\n", ecmap_size (map01));
  }
  
  ecmap_destroy (&map01);
  
  return 0;
}

//----------------------------------------------------------------------------------------

void* __STDCALL test_ecmap_test3_onCloneKey (void* ptr)
{
  void* key = malloc(42);

  memcpy (key, ptr, 42);
  
  return key;
}

//----------------------------------------------------------------------------------------

void* __STDCALL test_ecmap_test3_onCloneVal (void* ptr)
{
  void* val = malloc(42);
  
  memcpy (val, ptr, 42);
  
  return val;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecmap_test3 (void* ptr, TestEnvContext ctx, EcErr err)
{
  int i;
  EcMap map02;
  EcMap map01 = ecmap_create (NULL, test_stdlist_onItemDestroy);
  
  for (i = 0; i < 10; i++)
  {
    void* key = ecstr_create_fmt (42, "item [%i]", i);
    void* val = ecstr_create_fmt (42, "value [%i]", i);
    
    ecmap_insert(map01, key, val);
  }
  
  map02 = ecmap_clone(map01, test_ecmap_test3_onCloneKey, test_ecmap_test3_onCloneVal);
  
  // forward
  {
    EcMapCursor* cursor = ecmap_cursor_create (map02, LIST_DIR_NEXT);
    
    while (ecmap_cursor_next (cursor))
    {
      void* key = ecmap_node_key (cursor->node);
      void* val = ecmap_node_value (cursor->node);
      
      printf ("map item '%s':'%s'\n", key, val);
    }
    
    ecmap_cursor_destroy (&cursor);
  }
  
  ecmap_destroy (&map01);
  ecmap_destroy (&map02);
  
  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Test1", test_ecmap_init, test_ecmap_done, test_ecmap_test1);
  testenv_reg (te, "Test2", test_ecmap_init, test_ecmap_done, test_ecmap_test2);
  testenv_reg (te, "Test3", test_ecmap_init, test_ecmap_done, test_ecmap_test3);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
