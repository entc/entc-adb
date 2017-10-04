/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ecexec.h"

#include "../types/ecstream.h"

#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>

#define PIPE_READ   0
#define PIPE_WRITE  1

struct EcExec_s
{
  char* arguments[11];
  
  EcStream stdout;
  
  EcStream stderr;
  
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
  
  self->stdout = ecstream_new();
  self->stderr = ecstream_new();
  
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
  
  ecstream_delete(&(self->stdout));
  ecstream_delete(&(self->stderr));
  
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
  
  eclogger_fmt (LL_TRACE, "_SYS", "exec", "run '%s' '%s'", self->arguments[0], self->arguments[1]);    

  pid = fork();

  switch( pid )
  {
    case -1: // error
    {
      eclogger_errno (LL_ERROR, "_SYS", "exec", "fork failed");    
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
      
      ecstream_clear(self->stdout);
      ecstream_clear(self->stderr);
      
      close(outfd[PIPE_WRITE]); /* close writing ends */
      close(errfd[PIPE_WRITE]);
      
      //    FILE * pipes[2];
      
      //    pipes[0] = fdopen(outfd[PIPE_READ], "r"); /* return reading end to the caller */
      //    pipes[1] = fdopen(errfd[PIPE_READ], "r");
      
      fd_set fdsread;
      FD_ZERO(&fdsread);
      
      FD_SET(outfd[0], &fdsread);
      FD_SET(errfd[0], &fdsread);
      
      int res01;
      int res02;
      
      char buffer[21];
      
      FILE * stdout = fdopen(outfd[0], "r");
      FILE * stderr = fdopen(errfd[0], "r");
      
      do 
      {
        w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        
        res01 = select(errfd[PIPE_READ] + 1, &fdsread, NULL, NULL, NULL);
        
        if(FD_ISSET(outfd[PIPE_READ], &fdsread))
        {        
          res02 = fread(buffer, 1, 20, stdout);
          while(res02 > 0)
          {
            eclogger_fmt (LL_TRACE, "_SYS", "exec", "stdout: got data %i", res02);    
            
            ecstream_appendd (self->stdout, buffer, res02);
            res02 = fread(buffer, 1, 20, stdout);
          }
        }
        
        if(FD_ISSET(errfd[PIPE_READ], &fdsread))
        {
          res02 = fread(buffer, 1, 20, stderr);
          while(res02 > 0)
          {
            eclogger_fmt (LL_TRACE, "_SYS", "exec", "stderr: got data %i", res02);    

            ecstream_appendd (self->stderr, buffer, res02);
            res02 = fread(buffer, 1, 20, stderr);
          }
        }        
      }
      while (!WIFEXITED(status) && !WIFSIGNALED(status));
      
      eclogger_fmt (LL_TRACE, "_SYS", "exec", "run '%s' done", self->arguments[0]);    
      
      fclose(stdout);
      fclose(stderr);
      
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
  return ecstream_buffer(self->stdout);
}

//-----------------------------------------------------------------------------------

const EcString ecexec_stderr (EcExec self)
{
  return ecstream_buffer(self->stderr);
}

//-----------------------------------------------------------------------------------

#endif
