#include "ecproc.h"

// entc includes
#include "system/ecfile.h"
#include "sys/entc_mutex.h"
#include "types/ecbuffer.h"
#include "types/ecmap.h"
#include "tools/eclog.h"

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
   if (ecproc_isWinNT()) //initialize security descriptor (Windows NT)
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
  
  ecproc_initSecurityAttributes (&sa, &sd);
  
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

  if (self->pipe)
  {
    CloseHandle (self->pipe);
  }
  
  CloseHandle (self->processInfo.hProcess);
  
  ENTC_DEL (pself, struct EcProc_s);
}

//-----------------------------------------------------------------------------

int ecproc_createPipe (EcProc self, EcErr err)
{
  EcBuffer buf = ecbuf_create_uuid ();

  ecstr_replaceTO (&(self->pipeUUID), ecbuf_str (&buf));
  //ecstr_replaceTO (&(self->pipeName), ecstr_cat2 ("\\\\.\\pipe\\", self->pipeUUID));

  self->pipe = ecproc_createNamedPipe (self->pipeUUID, err);
  if (self->pipe == NULL)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecproc_command (EcProc self, const EcString folder, const EcString binary, const char* args, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcString realArgs;

  ZeroMemory (&(self->startupInfo), sizeof(STARTUPINFO));
  self->startupInfo.cb = sizeof(STARTUPINFO);

  ZeroMemory (&(self->processInfo), sizeof(PROCESS_INFORMATION));

  if (args == NULL)
  {
    // create a named pipe for communication
    res = ecproc_createPipe (self, err);
	if (res)
	{
      return res;
	}

    realArgs = ecstr_cat3 (binary, " ", self->pipeUUID);
  }
  else
  {
    realArgs = ecstr_cat3 (binary, " ", args);
  }

  {
    EcString pathbn = ecfs_mergeToPath (folder, binary);

    if (CreateProcessA (pathbn, realArgs, NULL, NULL, TRUE, 0, NULL, folder, &(self->startupInfo), &(self->processInfo)) == 0)
	{
      eclog_fmt (LL_ERROR, "Q6", "start proc", "can't start '%s' : '%s'", folder, binary);

	  res = ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
	}

    ecstr_delete (&pathbn);
  }

  ecstr_delete (&realArgs);

  if ((res == ENTC_ERR_NONE) && self->pipe)
  {
    if (ConnectNamedPipe (self->pipe, NULL) == 0)
    {

    }
  }

  return res;
}

//-----------------------------------------------------------------------------

int ecproc_start (EcProc self, const char* command, const char* args, EcErr err)
{
  int res;
  const EcString cmdName = ecfs_extractFile (command);

  // for windows
  EcString execbn = ecstr_cat2 (cmdName, ".exe");

  // use the current directory
  EcString folder = ecproc_ownPath ();
	
  res = ecproc_command (self, folder, execbn, args, err);

  ecstr_delete (&execbn);
  ecstr_delete (&folder);

  return res;
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

    void* pipeHandle = ecproc_openNamedPipe (argv[1], err);
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

	eclog_fmt (LL_WARN, "Q6_PROC", "terminate", "can't terminate '%s'", err->text);

	ecerr_destroy (&err);
  }
}

//-----------------------------------------------------------------------------

int ecproc_waitForProcessToTerminate (EcProc self, EcErr err)
{
  return ecproc_waitForProcess (self->processInfo.hProcess, err);
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

int ecproc_waitForProcess (void* handle, EcErr err)
{
  DWORD res = WaitForSingleObject (handle, INFINITE);
  
  

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

#endif
