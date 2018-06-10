#include "ecdaemon.h"

// entc includes
#include "system/macros.h"

#if defined _WIN64 || defined _WIN32

#include <windows.h>

//-----------------------------------------------------------------------------

struct EcDaemon_s
{
  EcString name;
  
  FILE* stream;

};

//-----------------------------------------------------------------------------

EcDaemon ecdaemon_create (const EcString name)
{
  EcDaemon self = ENTC_NEW (struct EcDaemon_s);
  
  self->name = ecstr_copy (name);
  self->stream = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecdaemon_delete (EcDaemon* pself)
{
  EcDaemon self = *pself;
  
  if (self->stream)
  {
    fclose (self->stream);
  }
  
  ecstr_delete (&(self->name));
  
  ENTC_DEL (pself, struct EcDaemon_s);
}

//-----------------------------------------------------------------------------

static ecdaemon_onShutdown onGlobalShutdown = NULL;
static void* globalPtr = NULL;

//-----------------------------------------------------------------------------

void WINAPI ecdaemon_serviceCtrlHandler (DWORD ctrl)
{
  switch (ctrl)
  {
    case SERVICE_CONTROL_STOP:
    {

      break;
    }
    case SERVICE_CONTROL_PAUSE:
    {

      break;
    }
    case SERVICE_CONTROL_CONTINUE:
    {

      break;
    }
    case SERVICE_CONTROL_SHUTDOWN:
    {
      EcErr err = ecerr_create ();
      
      int res = onGlobalShutdown (globalPtr, err);
      if (res)
      {
        
      }
      
      ecerr_destroy (&err);
      break;
    }
    case SERVICE_CONTROL_INTERROGATE:
    {

      break;
    }
  }
}

//-----------------------------------------------------------------------------

int ecdaemon_run (EcDaemon self, void* ptr, ecdaemon_onRun onRun, ecdaemon_onShutdown onShutdown, EcErr err)
{
  errno_t err;
  FILE* stream;
  
  RegisterServiceCtrlHandler (self->name, ecdaemon_serviceCtrlHandler);
  
  // Reassign "stderr" to "freopen.out":
  err = freopen_s (&(self->stream), "log.out", "w", stdout);
  if (err != 0)
  {
    if (onRun)
    {
      globalPtr = ptr;
      onGlobalShutdown = onShutdown;
      
      return onRun (ptr, err);
    }
    else
    {
      return ENTC_ERR_NONE;
    }
  }
  else
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
}

//-----------------------------------------------------------------------------

int ecdaemon_install (EcDaemon self, EcErr err)
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;
  TCHAR szPath[MAX_PATH];

  if(!GetModuleFileName (NULL, szPath, MAX_PATH ))
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 

  if (NULL == schSCManager) 
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  schService = CreateService (schSCManager, self->name, self->name, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, szPath, NULL, NULL, NULL, NULL, NULL);                     

  if (schService == NULL) 
  {
    CloseServiceHandle(schSCManager);
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  else
  {

  }

  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

#else

//-----------------------------------------------------------------------------

struct EcDaemon_s
{
  int dummy;
  
};

//-----------------------------------------------------------------------------

EcDaemon ecdaemon_create (const EcString name)
{
  EcDaemon self = ENTC_NEW (struct EcDaemon_s);
  
  return self;
}

//-----------------------------------------------------------------------------

void ecdaemon_delete (EcDaemon* pself)
{
  EcDaemon self = *pself;
  
  ENTC_DEL (pself, struct EcDaemon_s);
}

//-----------------------------------------------------------------------------

int ecdaemon_install (EcDaemon self, EcErr err)
{
  // not supported yet
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecdaemon_run (EcDaemon self, void* ptr, ecdaemon_onRun onRun, ecdaemon_onShutdown onShutdown, EcErr err)
{
  if (onRun)
  {
    return onRun (ptr, err);
  }
  else
  {
    return ENTC_ERR_NONE;
  }
}

//-----------------------------------------------------------------------------

#endif

