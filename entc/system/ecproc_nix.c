#include "ecproc.h"

// entc includes
#include "types/ecbuffer.h"
#include "system/ecfile.h"
#include "system/ecmutex.h"
#include "types/ecmap.h"
#include "utils/eclogger.h"
#include <system/ecaio_proc.h>

#if defined __BSD_OS || defined __LINUX_OS

#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

//-----------------------------------------------------------------------------

EcString ecproc_getExecutableName (int argc, char* argv[])
{
  if (argc > 0)
  {
    return ecfs_extractFileName (argv[0]);
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------

#define PIPE_READ   0
#define PIPE_WRITE  1

//-----------------------------------------------------------------------------

void ecproc_addParameter (char** arguments, const EcString param)
{
  int i;
  for (i = 1; i < 10; i++)
  {
    if (isNotAssigned(arguments[i]))
    {
      arguments[i] = ecstr_copy(param);
      arguments[i + 1] = NULL;
      return;
    }
  }
}

//=============================================================================

struct EcProc_s
{
  
  unsigned long pid;
  
  EcString fifoIn;
  
  EcString fifoOut;
  
  unsigned long fdIn;
  
  unsigned long fdOut;
  
};

//-----------------------------------------------------------------------------

EcProc ecproc_create (void)
{
  EcProc self = ENTC_NEW(struct EcProc_s);
  
  self->pid = 0;
  
  self->fifoIn = NULL;
  self->fifoOut = NULL;
  
  self->fdIn = 0;
  self->fdOut = 0;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecproc_closeWriting (EcProc self)
{
  if (self->fifoOut)
  {
    printf ("about to close writing\n");
    
    if (close (self->fdOut) < 0)
    {
      EcErr err = ecerr_create();
      
      ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
      
      eclogger_fmt (LL_ERROR, "ENTC PRC", "close write", "can't close pipe: %s", err->text);
      
      ecerr_destroy (&err);
    }
    
    printf ("writing closed\n");
  }
}

//-----------------------------------------------------------------------------

void ecproc_closeReading (EcProc self)
{
  if (self->fifoIn)
  {
    printf ("about to close reading\n");
    
    if (close (self->fdIn) < 0)
    {
      EcErr err = ecerr_create();
      
      ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
      
      eclogger_fmt (LL_ERROR, "ENTC PRC", "close read", "can't close pipe: %s", err->text);
      
      ecerr_destroy (&err);
    }
    
    printf ("reading closed\n");
  }
}

//-----------------------------------------------------------------------------

void ecproc_destroy (EcProc* pself)
{
  EcProc self = *pself;
  
  eclogger_fmt (LL_TRACE, "ENTC PRC", "destroy", "process context '%p' closing pipes", self);
  
  ecproc_closeWriting (self);
  //ecproc_closeReading (self);
  
  if (self->fifoIn)
  {
    eclogger_fmt (LL_TRACE, "ENTC PRC", "destroy", "remove FIFO IN %s", self->fifoIn);
 
    // remove the fifo file
    ecfs_rmfile (self->fifoIn);
    
    ecstr_delete(&(self->fifoIn));
  }
  
  if (self->fifoOut)
  {
    eclogger_fmt (LL_TRACE, "ENTC PRC", "destroy", "remove FIFO OUT %s", self->fifoOut);

    // remove the fifo file
    ecfs_rmfile (self->fifoOut);
    
    ecstr_delete(&(self->fifoOut));
  }
  
  eclogger_fmt (LL_TRACE, "ENTC PRC", "destroy", "process context '%p' destroyed", self);
 
  ENTC_DEL (pself, struct EcProc_s);
}

//-----------------------------------------------------------------------------------

EcString q6sys_procbroker_createFifo ()
{
  // new uuid as fifo name
  EcBuffer buf = ecbuf_create_uuid ();
  
  // merge to path
  EcString fifo = ecfs_mergeToPath ("/tmp", ecbuf_const_str(buf));
  
  // create the file
  mkfifo (fifo, 0666);
  
  return fifo;
}

//-----------------------------------------------------------------------------

int ecproc_start (EcProc self, const char* command, const char* args, EcErr err)
{
  if (args == NULL)
  {
    self->fifoIn = q6sys_procbroker_createFifo ();
    self->fifoOut = q6sys_procbroker_createFifo ();
  }
  
  // fork the process
  self->pid = fork();
  
  switch (self->pid)
  {
    case -1: // error
    {
      int ret = ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
      
      eclogger_fmt (LL_ERROR, "ENTC", "start proc", "can't fork: %s", err->text);
      
      return ret;
    }
    case  0: // child process
    {
      int i;
      char* arguments [11];

      arguments[0] = ecstr_copy (command);

      for (i = 1; i < 10; i++)
      {
        arguments[i] = NULL;
      }

      if (args)
      {
        ecproc_addParameter (arguments, args);
      }
      else
      {
        ecproc_addParameter (arguments, self->fifoOut);
        ecproc_addParameter (arguments, self->fifoIn);
      }
      
      eclogger_fmt (LL_DEBUG, "ENTC", "start proc", "%s | %s | %s", arguments[0], arguments[1], arguments[2]);
      
      execv(arguments[0], arguments);
      
      exit(0);
    }
    default: // parent
    {
      if (args == NULL)
      {
        // open FIFO (named pipe)
        self->fdIn = open (self->fifoIn, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (self->fdIn == -1)
        {
          return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
        }
        
        // open FIFO (named pipe)
        self->fdOut = open (self->fifoOut, O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (self->fdOut == -1)
        {
          return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
        }
      }
      
      return ENTC_ERR_NONE;
    }
  }
}

//-----------------------------------------------------------------------------

EcProc ecproc_get (int argc, char* argv[], EcErr err)
{
  int fdIn, fdOut;
  
  if (argc < 3)
  {
    eclogger_msg (LL_ERROR, "ENTC", "get proc", "no pipe");
    return NULL;
  }
  
  fdOut = open (argv[2], O_WRONLY);
  if( fdOut == -1 )
  {
    eclogger_fmt (LL_ERROR, "ENTC", "open pipe", "can't open pipe '%s'", argv[2]);
    return NULL;
  }
  
  fdIn = open (argv[1], O_RDONLY);
  if( fdIn == -1 )
  {
    eclogger_fmt (LL_ERROR, "ENTC", "open pipe", "can't open pipe '%s'", argv[1]);
    return NULL;
  }
  
  EcProc self = ecproc_create ();
  
  self->fdIn = fdIn;
  self->fdOut = fdOut;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecproc_terminate (EcProc self)
{
  if (self->pid)
  {
    kill(self->pid, SIGTERM);

    eclogger_fmt (LL_TRACE, "ENTC", "ecproc", "send terminate signal to [%i]", self->pid);
 
    ecproc_closeWriting (self);
  }
}

//-----------------------------------------------------------------------------

int ecproc_waitForProcessToTerminate (EcProc self, EcErr err)
{
  if (self->pid)
  {
    int res;
    
    eclogger_fmt (LL_TRACE, "ENTC PRC", "wait", "wait for process '%p'", self);
    
    ecproc_closeReading (self);
    
    res = ecproc_waitForProcess ((void*)self->pid, err);

    eclogger_fmt (LL_TRACE, "ENTC PRC", "wait", "wait for process '%p' done", self);
    
    return res;
  }
  else
  {
    return ENTC_ERR_NONE;
  }
}

//-----------------------------------------------------------------------------

int ecproc_waitForProcess (void* handle, EcErr err)
{
  int res;
 
  siginfo_t info;
  memset (&info, 0, sizeof(siginfo_t));
  
  res = waitid (P_ALL, (unsigned long)handle, &info, WUNTRACED | WEXITED);
  if (res < 0)
  {
    int errorCode = errno;
    if (errorCode == ECHILD)
    {
      // no process or already terminated
      eclogger_fmt (LL_WARN, "ENTC", "child", "child process not found [%lu]", (unsigned long)handle);

      return ENTC_ERR_NONE;
    }
    
    return ecerr_formatErrorOS (err, ENTC_LVL_ERROR, errorCode);
  }
  
  switch (info.si_code)
  {
    case CLD_EXITED:
    {
      eclogger_fmt (LL_WARN, "ENTC", "child", "child process terminated [%i]", info.si_pid);
      break;
    }
    case CLD_KILLED:
    {
      eclogger_fmt (LL_ERROR, "ENTC", "child", "child process killed [%i]", info.si_pid);
      break;
    }
    case CLD_DUMPED:
    {
      eclogger_fmt (LL_WARN, "ENTC", "child", "child process dumped [%i]", info.si_pid);
      break;
    }
    case CLD_STOPPED:
    {
      eclogger_fmt (LL_DEBUG, "ENTC", "child", "child process stopped [%i]", info.si_pid);
      break;
    }
    case CLD_TRAPPED:
    {
      eclogger_fmt (LL_DEBUG, "ENTC", "child", "child process trapped [%i]", info.si_pid);
      break;
    }
    case CLD_CONTINUED:
    {
      eclogger_fmt (LL_DEBUG, "ENTC", "child", "child process stopped [%i]", info.si_pid);
      break;
    }
    default:
    {
      eclogger_fmt (LL_ERROR, "ENTC", "child", "unknow reason for child process terminated [%i]", info.si_pid);
      break;
    }
  }

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void* ecproc_handle (EcProc self)
{
  return (void*)self->pid;
}

//-----------------------------------------------------------------------------

void* ecproc_getHandleIn (EcProc self)
{
  return (void*)self->fdIn;
}

//-----------------------------------------------------------------------------

void* ecproc_getHandleOut (EcProc self)
{
  return (void*)self->fdOut;
}

//-----------------------------------------------------------------------------

#endif
