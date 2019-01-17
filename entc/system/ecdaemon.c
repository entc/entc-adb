#include "ecdaemon.h"

// entc includes
#include "tools/eclog.h"
#include "system/ecfile.h"

#if defined _WIN64 || defined _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------

struct EcDaemon_s
{
  EcString exec;
  
  EcString path;

  EcString name;
  
  FILE* stream;

};

//-----------------------------------------------------------------------------

EcDaemon ecdaemon_create (int argc, char *argv[], const EcString name)
{
  EcDaemon self = ENTC_NEW (struct EcDaemon_s);

  self->path = NULL;
  self->exec = NULL;

  // fetch the path and the name from the current running process
  ecfs_getExecutable (&(self->path), &(self->exec), argc, argv);
  
  self->name = ecstr_copy (name);

  eclog_fmt (LL_TRACE, "ENTC SYS", "daemon", "create '%s' : '%s' -> '%s'", self->name, self->path, self->exec);

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
  ecstr_delete (&(self->path));
  ecstr_delete (&(self->exec));
  
  ENTC_DEL (pself, struct EcDaemon_s);
}

//-----------------------------------------------------------------------------

static ecdaemon_onRun onGlobalRun = NULL;
static ecdaemon_onShutdown onGlobalShutdown = NULL;
static void* globalPtr = NULL;
static const EcString srvName = NULL;

SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
SERVICE_STATUS        g_ServiceStatus = {0};

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

VOID WINAPI ecdaemon_service_main (DWORD argc, LPTSTR *argv)
{  
  g_StatusHandle = RegisterServiceCtrlHandler (srvName, ecdaemon_serviceCtrlHandler);
  if (g_StatusHandle == NULL) 
  {
    return;
  }

  // Tell the service controller we are starting
  ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
  g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwServiceSpecificExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;
 
  if (SetServiceStatus (g_StatusHandle , &g_ServiceStatus) == FALSE)
  {
    return;
  }

  // Tell the service controller we are started
  g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 0;
 
  if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
  {
    return;
  }

  if (onGlobalRun)
  {
    EcErr err = ecerr_create ();

	onGlobalRun (globalPtr, err);

	ecerr_destroy (&err);
  }

  // Tell the service controller we are stopped
  g_ServiceStatus.dwControlsAccepted = 0;
  g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
  g_ServiceStatus.dwWin32ExitCode = 0;
  g_ServiceStatus.dwCheckPoint = 3;
 
  if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
  {

  }
}

//-----------------------------------------------------------------------------

int ecdaemon_service (EcDaemon self, EcErr err)
{
  SERVICE_TABLE_ENTRY ServiceTable[] = {{self->name, (LPSERVICE_MAIN_FUNCTION) ecdaemon_service_main}, {NULL, NULL}};

  if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecdaemon_reroute_stdout (EcDaemon self, EcErr err)
{
  int res;
  EcString logFile;

  logFile = ecfs_mergeToPath (self->path, "q6daemon.log");

  // Reassign "stderr" to "freopen.out":
  res = freopen_s (&(self->stream), logFile, "w", stdout);

  ecstr_delete (&logFile);

  if (err != 0)
  {
	// change to real path
	return ecfs_chdir (self->path, err);
  }
  else
  {
	return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
}

//-----------------------------------------------------------------------------

int ecdaemon_run (EcDaemon self, void* ptr, ecdaemon_onRun onRun, ecdaemon_onShutdown onShutdown, EcErr err)
{
  int res;
  
  res = ecdaemon_reroute_stdout (self, err);
  if (res)
  {	
    return res;
  }

  globalPtr = ptr;
  onGlobalRun = onRun;
  onGlobalShutdown = onShutdown;
  srvName = self->name;

  return ecdaemon_service (self, err);
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

  eclog_fmt (LL_TRACE, "ENTC SYS", "daemon", "install as daemon '%s' -> '%s'", szPath, self->name);

  schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 

  if (NULL == schSCManager) 
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  schService = CreateService (schSCManager, self->name, self->name, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szPath, NULL, NULL, NULL, NULL, NULL);                     

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

EcDaemon ecdaemon_create (int argc, char *argv[], const EcString name)
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

