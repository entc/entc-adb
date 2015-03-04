#include "tools/ecasyncvc.h"
#include "system/ecsignal.h"

int main (int argc, char *argv[])
{
  EcAsyncServ serv;

  EcAsyncServCallbacks callbacks;
  callbacks.onCreate = NULL;
  callbacks.onDestroy = NULL;
  callbacks.onIdle = NULL;
  callbacks.onRecvAll = NULL;
  
  serv = ecaserv_create (&callbacks);
  
  ecaserv_start (serv, "127.0.0.1", 8080);
  
  ecsignal_init ( ecaserv_getEventContext (serv));
  
  ecaserv_run (serv);
  
  ecaserv_destroy (&serv);
  
  return 0;
}
