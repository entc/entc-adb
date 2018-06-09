#include "ecdaemon.h"

// entc includes
#include "types/ecstring.h"

#if defined _WIN64 || defined _WIN32

#include <windows.h>

//-----------------------------------------------------------------------------

struct EcDaemon_s
{
  EcString name;

};

//-----------------------------------------------------------------------------

EcDaemon ecdaemon_create ()
{

}

//-----------------------------------------------------------------------------

void ecdaemon_delete (EcDaemon* pself)
{

}

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

      break;
    }
    case SERVICE_CONTROL_INTERROGATE:
    {

      break;
    }
  }
}

//-----------------------------------------------------------------------------

void ecdaemon_run (EcDaemon self)
{
  RegisterServiceCtrlHandler (self->name, ecdaemon_serviceCtrlHandler);
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

#endif

