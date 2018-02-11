#include "ecexec.h"

#include "system/macros.h"
#include "types/ecstream.h"
#include "tools/eclog.h"

#if defined __BSD_OS || defined __LINUX_OS

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>

#define PIPE_READ   0
#define PIPE_WRITE  1

struct EcExec_s
{
  char* arguments[11];
  
  EcStream outstream;
  
  EcStream errstream;
  
};

//-----------------------------------------------------------------------------------

EcExec ecexec_new (const EcString script)
{
  int i;
  EcExec self = ENTC_NEW (struct EcExec_s);
  
  self->arguments[0] = ecstr_copy(script);
  
  for (i = 1; i < 10; i++) {
    self->arguments[i] = NULL;
  }
  
  self->outstream = ecstream_create ();
  self->errstream = ecstream_create ();
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecexec_delete (EcExec* pself)
{
  int i;
  EcExec self = *pself;

  for (i = 0; i < 10; i++)
  {
    ecstr_delete(&(self->arguments[i]));
  }
  
  ecstream_destroy (&(self->outstream));
  ecstream_destroy (&(self->errstream));
  
  ENTC_DEL (pself, struct EcExec_s);
}

//-----------------------------------------------------------------------------------

void ecexec_addParameter (EcExec self, const EcString param)
{
  int i;
  for (i = 1; i < 10; i++)
  {
    if (isNotAssigned(self->arguments[i]))
    {
      self->arguments[i] = ecstr_copy(param);
      self->arguments[i + 1] = NULL;
      return;
    }
  }
}

//-----------------------------------------------------------------------------------

int ecexec_run (EcExec self)
{
  pid_t pid;
  
  int outfd[2], errfd[2];
  if (pipe(outfd) == -1 || pipe(errfd) == -1) return -1;
  ///maybe here thread instead of fork  
  
  eclog_fmt (LL_TRACE, "_SYS", "exec", "run '%s' '%s'", self->arguments[0], self->arguments[1]);    

  pid = fork();

  switch( pid )
  {
    case -1: // error
    {
      eclog_err_os (LL_ERROR, "_SYS", "exec", "fork failed");    
    }
    break;
    case  0: // child process
    { 
      /* child process */
      close(outfd[PIPE_READ]); /* close reading end */
      close(errfd[PIPE_READ]);
      
      //redirecting stdout
      dup2(outfd[PIPE_WRITE], STDOUT_FILENO);
      //redirecting stderr
      dup2(errfd[PIPE_WRITE], STDERR_FILENO);
      
      execv(self->arguments[0], self->arguments);
      
      close(outfd[PIPE_WRITE]);
      close(errfd[PIPE_WRITE]);
      
      exit(0);
    }
    break;
    default: // parent
    {
      int status = 0;
      pid_t w;
      
      fd_set fdsread;
      FD_ZERO(&fdsread);

      ecstream_clear(self->outstream);
      ecstream_clear(self->errstream);
      
      close(outfd[PIPE_WRITE]); /* close writing ends */
      close(errfd[PIPE_WRITE]);
      
      FD_SET(outfd[0], &fdsread);
      FD_SET(errfd[0], &fdsread);
      
      int res01;
      int res02;
      
      char buffer[21];
      
      FILE* outf = fdopen(outfd[0], "r");
      FILE* errf = fdopen(errfd[0], "r");
      
      do 
      {
        w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        
        res01 = select(errfd[PIPE_READ] + 1, &fdsread, NULL, NULL, NULL);
        
        if(FD_ISSET(outfd[PIPE_READ], &fdsread))
        {        
          res02 = fread(buffer, 1, 20, outf);
          while(res02 > 0)
          {
            eclog_fmt (LL_TRACE, "_SYS", "exec", "stdout: got data %i", res02);    
            
            ecstream_append_buf (self->outstream, buffer, res02);
            res02 = fread(buffer, 1, 20, outf);
          }
        }
        
        if(FD_ISSET(errfd[PIPE_READ], &fdsread))
        {
          res02 = fread(buffer, 1, 20, errf);
          while(res02 > 0)
          {
            eclog_fmt (LL_TRACE, "_SYS", "exec", "stderr: got data %i", res02);    

            ecstream_append_buf (self->errstream, buffer, res02);
            res02 = fread(buffer, 1, 20, errf);
          }
        }        
      }
      while (!WIFEXITED(status) && !WIFSIGNALED(status));
      
      eclog_fmt (LL_TRACE, "_SYS", "exec", "run '%s' done", self->arguments[0]);    
      
      fclose(outf);
      fclose(errf);
      
      close(outfd[PIPE_READ]);
      close(errfd[PIPE_READ]);

      return status;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------------

const EcString ecexec_stdout (EcExec self)
{
  return ecstream_get (self->outstream);
}

//-----------------------------------------------------------------------------------

const EcString ecexec_stderr (EcExec self)
{
  return ecstream_get (self->errstream);
}

//-----------------------------------------------------------------------------------

#endif
