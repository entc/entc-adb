#include "tools/ecserver.h"
#include "system/ecsignal.h"

//-------------------------------------------------------------------------

int _STDCALL test_accept (void* ptr, void** object)
{
  EcSocket self = ptr;
  
  EcSocket socket;
  
  if(self == 0)
  {
    return FALSE;
  }
  
  socket = ecsocket_acceptIntr (self);
  
  if(!socket)
  {
    return FALSE;
  }
  
  *object = socket;
  
  return TRUE;
}

//-------------------------------------------------------------------------

int _STDCALL test_worker (void* ptr, void** object)
{
  // casts
  EcSocket socket = (EcSocket)*object;
  
  
  ecsocket_delete(&socket);
  *object = NULL;
  
  // signaled false to stop the thread  
  return TRUE;   
}

//-------------------------------------------------------------------------

int _STDCALL test_worker_clear (void* ptr, void** object)
{
  EcSocket socket = (EcSocket)*object;
  
  ecsocket_delete(&socket);
  *object = NULL;
  
  return TRUE;
}

//-------------------------------------------------------------------------------------

int main (int argc, char *argv[])
{
  EcServerCallbacks callbacks;
  EcServer serv;

  EcEventContext ec = ece_context_new ();
  
  EcSocket socket = ecsocket_new (ec, ENTC_SOCKET_PROTOCOL_TCP);
  if (!ecsocket_listen (socket, "127.0.0.1", 8080))
  {
    ecsocket_delete(&socket);
    return 1;
  }
  
  callbacks.accept_thread = test_accept;
  callbacks.accept_ptr = socket;
  callbacks.worker_thread = test_worker;
  callbacks.worker_ptr = socket;
  callbacks.clear_fct = test_worker_clear;
  callbacks.clear_ptr = socket;  
  
  serv = ecserver_create (10, &callbacks, ec);
    

  ecserver_start(serv);
  
  ecsignal_init ( ec);
  
  ecserver_destroy (&serv);
  
  ecsocket_delete(&socket);
  
  ece_context_delete(&ec);
  
  return 0;
}
