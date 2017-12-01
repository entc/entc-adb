#include "ecproc.h"

// entc includes
#include "types/ecbuffer.h"
#include "system/ecfile.h"
#include "system/ecmutex.h"
#include "types/ecmap.h"
#include "utils/eclogger.h"

#if defined __WIN_OS

#include <windows.h>

//-----------------------------------------------------------------------------

int ecproc_isWinNT ()
{
  OSVERSIONINFO osv;

  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx (&osv);

  return (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

//-----------------------------------------------------------------------------

void ecproc_initSecurityAttributes (SECURITY_ATTRIBUTES* sa, SECURITY_DESCRIPTOR* sd)
{
   if (q6sys_proc_isWinNT()) //initialize security descriptor (Windows NT)
   {
      InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
      SetSecurityDescriptorDacl (sd, TRUE, NULL, FALSE);
      sa->lpSecurityDescriptor = sd;
   }
   else
   {
      sa->lpSecurityDescriptor = NULL;
   }

   sa->bInheritHandle = TRUE;
   sa->nLength = sizeof (SECURITY_ATTRIBUTES );
}

//-----------------------------------------------------------------------------

void* ecproc_createNamedPipe (const char* name, EcErr err)
{
  HANDLE pipeHandle;
  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;
  
  EcString namedPipe = ecstr_cat2 ("\\\\.\\pipe\\", name);
  
  q6sys_proc_initSecurityAttributes (&sa, &sd);
  
  pipeHandle = CreateNamedPipe (namedPipe, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE, 10, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, &sa);
  
  ecstr_delete (&namedPipe);

  if (pipeHandle == INVALID_HANDLE_VALUE)
  {
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    return NULL;
  }
  
  FlushFileBuffers (pipeHandle);
  
  return (void*)pipeHandle;
}

//-----------------------------------------------------------------------------

void* ecproc_openNamedPipe (const char* name, EcErr err)
{
  EcString namedPipe = ecstr_cat2 ("\\\\.\\pipe\\", name);
  
  HANDLE pipeHandle = CreateFile (namedPipe, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
  
  ecstr_delete (&namedPipe);
  
  if (pipeHandle == INVALID_HANDLE_VALUE)
  {
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    return NULL;
  }
  
  return (void*)pipeHandle;
}

//-----------------------------------------------------------------------------

char* ecproc_ownPath ()
{
  EcString executable;
  EcString exepath;
  
  EcBuffer buf = ecbuf_create (MAX_PATH);
  
  GetModuleFileNameA (NULL, buf->buffer, buf->size);
  
  executable = ecbuf_str (&buf);
  
  exepath = ecfs_getDirectory (executable);
  
  ecstr_delete (&executable);
  
  return exepath;
}

//-----------------------------------------------------------------------------

struct EcProc_s
{

  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;

  STARTUPINFO startupInfo;
  PROCESS_INFORMATION processInfo;

  HANDLE pipe;

  EcString pipeUUID;

  //EcString pipeName;

};

//-----------------------------------------------------------------------------

EcProc ecproc_create (void)
{
  EcProc self = ENTC_NEW (struct EcProc_s);

  self->pipe = NULL;
  self->pipeUUID = NULL;
  //self->pipeName = NULL;

  return self;
}

//-----------------------------------------------------------------------------

void ecproc_destroy (EcProc* pself)
{
  EcProc self = *pself;

  CloseHandle (self->pipe);
  
  CloseHandle (self->processInfo.hProcess);
  
  ENTC_DEL (pself, struct EcProc_s);
}

//-----------------------------------------------------------------------------

int ecproc_start (EcProc self, const char* command, const char* args, const char* folder, EcErr err)
{
  EcBuffer buf = ecbuf_create_uuid ();

  ecstr_replaceTO (&(self->pipeUUID), ecbuf_str (&buf));
  //ecstr_replaceTO (&(self->pipeName), ecstr_cat2 ("\\\\.\\pipe\\", self->pipeUUID));

  self->pipe = q6sys_pipe_createNamedPipe (self->pipeUUID, err);
  if (self->pipe == NULL)
  {
    return ENTC_ERR_OS_ERROR;
  }
  
  /*
  q6sys_proc_initSecurityAttributes (&(self->sa), &(self->sd));

  self->pipe = CreateNamedPipe(self->pipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE, 10, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, &(self->sa));
  if (self->pipe == INVALID_HANDLE_VALUE)
  {
    return q6err_lastErrorOS (err, Q6LVL_ERROR);
  }

  FlushFileBuffers (self->pipe);
*/
  
  
   
  ZeroMemory (&(self->startupInfo), sizeof(STARTUPINFO));
  self->startupInfo.cb = sizeof(STARTUPINFO);

  ZeroMemory (&(self->processInfo), sizeof(PROCESS_INFORMATION));

  {
    const EcString cmdName = ecfs_extractFile (command);
    EcString h2 = ecstr_cat3 (cmdName, " ", self->pipeUUID);
    EcString realArgs = ecstr_cat3 (h2, " ", args);

    int res = CreateProcessA (command, realArgs, NULL, NULL, TRUE, 0, NULL, folder, &(self->startupInfo), &(self->processInfo));

    ecstr_delete (&h2);
    ecstr_delete (&realArgs);

    if (res == 0)
    {
      eclogger_fmt (LL_ERROR, "Q6", "start proc", "can't start '%s'", command);

      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
  }

  {
    DWORD res = ConnectNamedPipe (self->pipe, NULL);
    if (res == FALSE)
    {

    }
  }
            
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

EcProc ecproc_get (int argc, char* argv[], EcErr err)
{
  if (argc < 2)
  {
    
  }
  else
  {
    EcProc process;

    void* pipeHandle = q6sys_pipe_openNamedPipe (argv[1], err);
    if (pipeHandle == INVALID_HANDLE_VALUE)
    {
      ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
      return NULL;
    }
    
    process = ecproc_create ();

    process->pipe = pipeHandle;
    process->pipeUUID = ecstr_copy (argv[1]);
      
    return process;
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------

void ecproc_terminate (EcProc self)
{
  UINT uExitCode = 0;
  
  if (TerminateProcess (self->processInfo.hProcess, uExitCode) == 0)
  {
    EcErr err = ecerr_create ();

    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);

	eclogger_fmt (LL_WARN, "Q6_PROC", "terminate", "can't terminate '%s'", err->text);

	ecerr_destroy (&err);
  }
}

//-----------------------------------------------------------------------------

int ecproc_waitForProcessToTerminate (EcProc self, EcErr err)
{
  DWORD res = WaitForSingleObject (self->processInfo.hProcess, INFINITE);
  
  
}

//-----------------------------------------------------------------------------

void* ecproc_handle (EcProc self)
{
  return self->processInfo.hProcess;
}

//-----------------------------------------------------------------------------

void* ecproc_getHandleIn (EcProc self)
{
  return self->pipe;
}

//-----------------------------------------------------------------------------

void* ecproc_getHandleOut (EcProc self)
{
  return self->pipe;
}

//-----------------------------------------------------------------------------

EcString ecproc_getExecutableName (int argc, char* argv[])
{
  char buffer [MAX_PATH];
  GetModuleFileNameA (NULL, buffer, MAX_PATH);
  
  return ecfs_extractFileName (buffer);
}

//-----------------------------------------------------------------------------

#endif
