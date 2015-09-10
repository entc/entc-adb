#include "tools/ecasyncvc.h"
#include "system/ecsignal.h"
#include "utils/ecmessages.h"

//-------------------------------------------------------------------------------------------

void _STDCALL ecasync_onData (void* ptr, const unsigned char* buffer, ulong_t len)
{
  printf ("data: '%s'", buffer);
}

//-------------------------------------------------------------------------------------------

EcAsyncContext _STDCALL ecasync_onAccept (void* ptr, EcSocket sock)
{
  return ecasync_worker_create (sock, 30000, ecasync_onData, NULL);
}

//-------------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  ecmessages_initialize ();
  
  EcEventContext ec = ece_context_new ();
  
  EcAsync async = ecasync_create (10);
  
  EcAsyncContext accept_ctx = ecasync_accept_create ("127.0.0.1", 8080, ec, async, ecasync_onAccept, NULL);
  
  ecasync_addToAll (async, &accept_ctx);
  
  ece_context_waitforAbort (ec, ENTC_INFINITE);
  
  ecasync_destroy (&async);
  
  return 0;
}
