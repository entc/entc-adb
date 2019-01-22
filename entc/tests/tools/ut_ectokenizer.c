#include "tools/ectokenizer.h"
#include "tests/ecenv.h"

#include <stdio.h>

//=============================================================================

static int __STDCALL test_tokenizer_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  {
    const EcString s1 = "213das\nhgsadj\n\t\ndshsahd\n7634ahdg\n\nn\n";

    EntcList tokens = ectokenizer_token (s1, "\n");
    
    EntcListCursor cursor;
    
    for (entc_list_cursor_init (tokens, &cursor, ENTC_DIRECTION_FORW); entc_list_cursor_next (&cursor);)
    {
      const EcString h = entc_list_node_data(cursor.node);
      
      printf ("[%i] : [%s]\n", cursor.position, h);
    }

    entc_list_del(&tokens);
  }
  {
    const EcString s1 = "213#das#nhgsadj#n##nndshsahd#n7634ahdg#n#nn#n";

    EntcList tokens = ectokenizer_token (s1, "#n");
    
    EntcListCursor cursor;
    
    for (entc_list_cursor_init (tokens, &cursor, ENTC_DIRECTION_FORW); entc_list_cursor_next (&cursor);)
    {
      const EcString h = entc_list_node_data(cursor.node);
      
      printf ("[%i] : [%s]\n", cursor.position, h);
    }

    entc_list_del(&tokens);
  }
  
  return 0;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  int i;
     
  testenv_reg (te, "Json Reader Test11", NULL, NULL, test_tokenizer_test1);

  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------

