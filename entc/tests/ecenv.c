#include "ecenv.h"

#include "types/eclist.h"
#include "utils/eclogger.h"
#include "utils/ecmessages.h"

// c libs includes
#include <stdio.h>

//-----------------------------------------------------------------------------

struct TestEnvContext_s
{
  
  fct_testenv_init init;
  
  fct_testenv_done done;
  
  fct_testenv_test test;
  
  void* ptr;
  
  const char* name;
  
  EcList cmpTextes;
  
  EcList errors;
  
}; typedef struct TestEnvContext_s* TestEnvContext;

//-----------------------------------------------------------------------------

void testctx_destroy (TestEnvContext* pself)
{
  TestEnvContext self = *pself;
  
  eclist_destroy (&(self->cmpTextes));
  eclist_destroy (&(self->errors));
  
  free (self);
}

//-----------------------------------------------------------------------------

static int __STDCALL testenv_cmpTextes_onDestroy (void* ptr)
{
  return 0;
}

//-----------------------------------------------------------------------------

static int __STDCALL testenv_errors_onDestroy (void* ptr)
{
  return 0;
}

//-----------------------------------------------------------------------------

void testctx_push_string (TestEnvContext self, const char* text)
{
  eclist_push_back (self->cmpTextes, (void*)text);
}

//-----------------------------------------------------------------------------

int testctx_pop_tocomp (TestEnvContext self, const char* text)
{
  int ret;
  char* orig = eclist_pop_front (self->cmpTextes);
  
  if (orig)
  {
    ret = ecstr_equal(orig, text);
  }
  else
  {
    ret = orig == text;
  }
  
  testenv_cmpTextes_onDestroy (orig);
  
  /*
  if (!ret)
  {
    printf ("comp missmatch '%s' <--> '%s'\n", orig, text);
  }
   */
  
  return ret;
}

//-----------------------------------------------------------------------------

void testctx_assert (TestEnvContext self, int cres, const char* comment)
{
  if (!cres)
  {
    // post some error
    eclist_push_back (self->errors, (void*)comment);
  }
}

//=============================================================================

struct TestEnv_s
{
  int max;
  
  TestEnvContext ctx[10];
  
  int errors;
  
};

//-----------------------------------------------------------------------------

TestEnv testenv_create ()
{
  TestEnv self = (TestEnv)malloc (sizeof(struct TestEnv_s));
  
  self->max = 0;
  self->errors = 0;
  
  ecmessages_initialize ();
  
  return self;
}

//-----------------------------------------------------------------------------

int testenv_destroy (TestEnv* pself)
{
  TestEnv self = *pself;
  int i;
  int r = self->errors;
  
  for (i = 0; i < self->max; i++)
  {
    TestEnvContext ctx = self->ctx[i];
    
    if (ctx)
    {
      testctx_destroy (&ctx);
    }
  }
  
  self->max = 0;
  
  free (self);
  *pself = NULL;
  
  ecmessages_deinitialize ();
  
  return r;
}

//-----------------------------------------------------------------------------

void testenv_run (TestEnv self)
{
  int i;
  
  // init
  for (i = 0; i < self->max; i++)
  {
    TestEnvContext ctx = self->ctx[i];
    
    ctx->ptr = ctx->init ();
  }
  
  // run test
  for (i = 0; i < self->max; i++)
  {
    int res;
    unsigned long errors;
    TestEnvContext ctx = self->ctx[i];
    
    eclogger_fmt (LL_INFO, "TEST", "start", "****");
    
    res = ctx->test (ctx->ptr, ctx);
    if (res)
    {
      
    }
    
    errors = eclist_size (ctx->errors);
    
    if (errors > 0)
    {
      EcListCursor cursor;
      
      eclist_cursor_init (ctx->errors, &cursor, LIST_DIR_NEXT);
      
      while (eclist_cursor_next (&cursor))
      {
        eclogger_fmt (LL_ERROR, "TEST", "error", eclist_data (cursor.node));
      }
    }
    else
    {
      eclogger_fmt (LL_INFO, "TEST", "done", "OK");
    }
  }
  
  // done
  for (i = 0; i < self->max; i++)
  {
    TestEnvContext ctx = self->ctx[i];
    
    ctx->done (ctx->ptr);
  }
}

//-----------------------------------------------------------------------------

void testenv_reg (TestEnv self, const char* name, fct_testenv_init init, fct_testenv_done done, fct_testenv_test test)
{
  TestEnvContext ctx = (TestEnvContext)malloc(sizeof(struct TestEnvContext_s));
  
  ctx->init = init;
  ctx->done = done;
  ctx->test = test;
  ctx->ptr = NULL;
  
  ctx->name = name;
  
  ctx->cmpTextes = eclist_create (testenv_cmpTextes_onDestroy);
  ctx->errors = eclist_create (testenv_errors_onDestroy);
  
  self->ctx[self->max] = ctx;
  self->max++;
}

//-----------------------------------------------------------------------------
