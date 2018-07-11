#include "adbo2.h"

// entc includes
#include <tests/ecenv.h>
#include <tools/ecjson.h>
#include <tools/eclog.h>
#include <system/ecthread.h>

//=============================================================================

static void* __STDCALL main_test_init ()
{
  EcString path = ecfs_getCurrentDirectory();
  
  Adbo2 adbo = adbo2_create (path, path, "adbo");

  ecstr_delete (&path);

  return adbo;
}

//---------------------------------------------------------------------------

static void __STDCALL main_test_done (void* ptr)
{
  Adbo2 adbo = ptr;

  adbo2_destroy (&adbo);
}

//---------------------------------------------------------------------------

static int __STDCALL main_test (void* ptr, TestEnvContext ectx, EcErr err)
{
  Adbo2 adbo = ptr;

  Adbo2Session session = adbo2_session_get (adbo, NULL, NULL);
  if (session)
  {
    Adbo2Transaction trx = adbo2_session_transaction (session);
    
    // query
    {
      EcUdc data = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, NULL);
      EcUdc para = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      ecudc_add_asNumber (EC_ALLOC, para, "id", 1);
      
      int res = adbo2_trx_query (trx, "test_table01", para, data, err);
      if (res)
      {
        return res;
      }
      
      {
        EcString h = ecjson_toString (data);

        eclog_fmt (LL_TRACE, "ADBO_2", "cursor", "got data %s", h);

        ecstr_delete (&h);
      }        
   
      ecudc_destroy (EC_ALLOC, &para);
      ecudc_destroy (EC_ALLOC, &data);
    }
    
    // query
    {
      EcUdc data = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, NULL);
      EcUdc para = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      ecudc_add_asNumber (EC_ALLOC, para, "fk01", 3);
      
      int res = adbo2_trx_query (trx, "test_table01", para, data, err);
      if (res)
      {
        return res;
      }
      
      {
        EcString h = ecjson_toString (data);

        eclog_fmt (LL_TRACE, "ADBO_2", "cursor", "got data %s", h);

        ecstr_delete (&h);
      }        
   
      ecudc_destroy (EC_ALLOC, &para);
      ecudc_destroy (EC_ALLOC, &data);
    }
    
    adbo2_trx_commit (&trx);
    
    adbo2_session_destroy (&session);
  }
    

  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL main_test_worker (void* ptr)
{
  Adbo2 adbo = ptr;
  int i;
  
  Adbo2Session session = adbo2_session_get (adbo, NULL, NULL);
  if (session)
  {
    Adbo2Transaction trx = adbo2_session_transaction (session);
    
    for (i = 0; i < 20; i++)
    {
      EcUdc data = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, NULL);
      EcUdc para = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
      
      ecudc_add_asNumber (EC_ALLOC, para, "fk01", 3);
      
      int res = adbo2_trx_query (trx, "test_table01", para, data, NULL);
      
      {
        EcString h = ecjson_toString (data);

        eclog_fmt (LL_TRACE, "ADBO_2", "cursor", "got data %s", h);

        ecstr_delete (&h);
      }        
   
      ecudc_destroy (EC_ALLOC, &para);
      ecudc_destroy (EC_ALLOC, &data);
    }

    adbo2_trx_commit (&trx);
  }
  
  adbo2_session_destroy (&session);
  
  return 0;
}

//---------------------------------------------------------------------------

#define MAX_THREADS 2

static int __STDCALL main_test_threading (void* ptr, TestEnvContext ectx, EcErr err)
{
  int i;
  EcThread threads [MAX_THREADS];
 
  for (i = 0; i < MAX_THREADS; i++)
  {
    threads [i] = ecthread_new (NULL);
    
    ecthread_start (threads [i], main_test_worker, ptr);
  }
  
  for (i = 0; i < MAX_THREADS; i++)
  {
    ecthread_join (threads [i]);
    
    ecthread_delete (&threads [i]);
  }

  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Main", main_test_init, main_test_done, main_test);
  testenv_reg (te, "Main", main_test_init, main_test_done, main_test_threading);

  testenv_run (te);
  
  return testenv_destroy (&te);
}

//-----------------------------------------------------------------------------

