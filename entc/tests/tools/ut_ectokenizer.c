#include "tools/ectokenizer.h"
#include "tests/ecenv.h"

//=============================================================================

static int __STDCALL test_tokenizer_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  {
    const EcString s1 = "213das\nhgsadj\n\t\ndshsahd\n7634ahdg\n\nn\n";

    EcList tokens = ectokenizer_token (s1, "\n");
    
    EcListCursor cursor;
    
    for (eclist_cursor_init (tokens, &cursor, LIST_DIR_NEXT); eclist_cursor_next (&cursor);)
    {
      const EcString h = eclist_data(cursor.node);
      
      printf ("[%i] : [%s]\n", cursor.position, h);
    }

    eclist_destroy(&tokens);
  }
  {
    const EcString s1 = "213#das#nhgsadj#n##nndshsahd#n7634ahdg#n#nn#n";

    EcList tokens = ectokenizer_token (s1, "#n");
    
    EcListCursor cursor;
    
    for (eclist_cursor_init (tokens, &cursor, LIST_DIR_NEXT); eclist_cursor_next (&cursor);)
    {
      const EcString h = eclist_data(cursor.node);
      
      printf ("[%i] : [%s]\n", cursor.position, h);
    }

    eclist_destroy(&tokens);
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

