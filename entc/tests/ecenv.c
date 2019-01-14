#include "ecenv.h"

#include "system/macros.h"
#include "tools/eclog.h"

// entc includes
#include "stc/entc_list.h"

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
  
  EntcList cmpTextes;
  
  EntcList errors;
  
}; typedef struct TestEnvContext_s* TestEnvContext;

//-----------------------------------------------------------------------------

void testctx_destroy (TestEnvContext* pself)
{
  TestEnvContext self = *pself;
  
  entc_list_del (&(self->cmpTextes));
  entc_list_del (&(self->errors));
  
  ENTC_DEL (pself, struct TestEnvContext_s);
}

//-----------------------------------------------------------------------------

static void __STDCALL testenv_cmpTextes_onDestroy (void* ptr)
{
}

//-----------------------------------------------------------------------------

static void __STDCALL testenv_errors_onDestroy (void* ptr)
{
  EcString h = ptr;
  ecstr_delete(&h);
}

//-----------------------------------------------------------------------------

void testctx_push_string (TestEnvContext self, const char* text)
{
  entc_list_push_back (self->cmpTextes, (void*)text);
}

//-----------------------------------------------------------------------------

int testctx_pop_tocomp (TestEnvContext self, const char* text)
{
  int ret;
  char* orig = entc_list_pop_front (self->cmpTextes);
  
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
    entc_list_push_back (self->errors, (void*)ecstr_copy(comment));
  }
}

//-----------------------------------------------------------------------------

int testctx_err (TestEnvContext self, EcErr err)
{
  if (err->code)
  {
    // post some error
    entc_list_push_back (self->errors, (void*)ecstr_copy(err->text));
  }

  return err->code;
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
  TestEnv self = ENTC_NEW (struct TestEnv_s);
  
  self->max = 0;
  self->errors = 0;
  
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
  
  return r;
}

//-----------------------------------------------------------------------------

void testenv_run (TestEnv self)
{
  int i;
  
  // run test
  for (i = 0; i < self->max; i++)
  {
    int res;
    unsigned long errors;
    TestEnvContext ctx = self->ctx[i];
    
    EcErr err = ecerr_create();
    
    if (ctx->init)
    {
      ctx->ptr = ctx->init (err);
    }
    else
    {
      ctx->ptr = NULL;
    }
    
    res = testctx_err (ctx, err);
    
    if (!res)
    {
      eclog_fmt (LL_INFO, "TEST", "start", "****");
      
      res = ctx->test (ctx->ptr, ctx, err);
      if (res)
      {
        // record this error
        testctx_err (ctx, err);
      }
    }
    
    errors = entc_list_size (ctx->errors);
    
    if (errors > 0)
    {
      EntcListCursor cursor;
      
      entc_list_cursor_init (ctx->errors, &cursor, ENTC_DIRECTION_FORW);
      
      while (entc_list_cursor_next (&cursor))
      {
        eclog_fmt (LL_ERROR, "TEST", "error", entc_list_node_data (cursor.node));
      }
    }
    else
    {
      eclog_fmt (LL_INFO, "TEST", "done", "OK");
    }
    
    if (!res && ctx->done)
    {
      ctx->done (ctx->ptr);
    }
    
    ecerr_destroy (&err);
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
  
  ctx->cmpTextes = entc_list_new (testenv_cmpTextes_onDestroy);
  ctx->errors = entc_list_new (testenv_errors_onDestroy);
  
  self->ctx[self->max] = ctx;
  self->max++;
}

//-----------------------------------------------------------------------------
