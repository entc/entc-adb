
#include "tools/eclparser.h"
#include "tests/ecenv.h"

//=============================================================================

typedef struct
{
  
  EcLineParser slp;
  
  TestEnvContext tctx;
  
} TestStdlparserCtx;

//---------------------------------------------------------------------------

static void __STDCALL onLine (void* ptr, const char* line)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)ptr;
  
  testctx_assert (ctx->tctx, testctx_pop_tocomp (ctx->tctx, line), "line compare");
}

//---------------------------------------------------------------------------

static void* __STDCALL test_stdlparser_init (EcErr err)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)malloc (sizeof(TestStdlparserCtx));
  
  ctx->slp = eclineparser_create (onLine, ctx);
  ctx->tctx = NULL;
  
  return ctx;
}

//---------------------------------------------------------------------------

static void __STDCALL test_stdlparser_done (void* ptr)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)ptr;
  
  eclineparser_destroy (&(ctx->slp));
  
  free (ctx);
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlparser_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)ptr;
  
  // test1
  const char* buf1 = "Hello\rWorld\rThanks!!";
  
  // should be the result in lines
  testctx_push_string (tctx, "Hello");
  testctx_push_string (tctx, "World");
  testctx_push_string (tctx, "Thanks!!");
  
  ctx->tctx = tctx;
  
  EcLineParser slp = ctx->slp;
  
  eclineparser_parse (slp, buf1, 10);
  eclineparser_parse (slp, buf1 + 10, 20);
  
  eclineparser_done (slp);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlparser_test2 (void* ptr, TestEnvContext tctx, EcErr err)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)ptr;
  
  // test1
  const char* buf1 = "Hello\nWorld\n\n\nThanks!!\n";
  
  // should be the result in lines
  testctx_push_string (tctx, "Hello");
  testctx_push_string (tctx, "World");
  testctx_push_string (tctx, NULL);
  testctx_push_string (tctx, NULL);
  testctx_push_string (tctx, "Thanks!!");
  testctx_push_string (tctx, NULL);
  
  ctx->tctx = tctx;
  
  EcLineParser slp = ctx->slp;
  
  eclineparser_parse (slp, buf1, 10);
  eclineparser_parse (slp, buf1 + 10, 20);
  
  eclineparser_done (slp);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlparser_test3 (void* ptr, TestEnvContext tctx, EcErr err)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)ptr;
  
  // test1
  const char* buf1 = "Hello\r\nWorld\r\n\r\nThanks!!\r\n";
  
  // should be the result in lines
  testctx_push_string (tctx, "Hello");
  testctx_push_string (tctx, "World");
  testctx_push_string (tctx, NULL);
  testctx_push_string (tctx, "Thanks!!");
  testctx_push_string (tctx, NULL);
  
  ctx->tctx = tctx;
  
  EcLineParser slp = ctx->slp;
  
  eclineparser_parse (slp, buf1, 10);
  eclineparser_parse (slp, buf1 + 10, 20);
  
  eclineparser_done (slp);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_stdlparser_test4 (void* ptr, TestEnvContext tctx, EcErr err)
{
  TestStdlparserCtx* ctx = (TestStdlparserCtx*)ptr;
  
  // test1
  const char* buf1 = "Hello\r\nWorld\r\r\nThanks!!\n";
  
  // should be the result in lines
  testctx_push_string (tctx, "Hello");
  testctx_push_string (tctx, "World");
  testctx_push_string (tctx, NULL);
  testctx_push_string (tctx, "Thanks!!");
  testctx_push_string (tctx, NULL);
  
  ctx->tctx = tctx;
  
  EcLineParser slp = ctx->slp;
  
  eclineparser_parse (slp, buf1, 10);
  eclineparser_parse (slp, buf1 + 10, 20);
  
  eclineparser_done (slp);
  
  return 0;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "LineParser Test1", test_stdlparser_init, test_stdlparser_done, test_stdlparser_test1);
  testenv_reg (te, "LineParser Test2", test_stdlparser_init, test_stdlparser_done, test_stdlparser_test2);
  testenv_reg (te, "LineParser Test3", test_stdlparser_init, test_stdlparser_done, test_stdlparser_test3);
  testenv_reg (te, "LineParser Test4", test_stdlparser_init, test_stdlparser_done, test_stdlparser_test4);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
