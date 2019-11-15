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

#if defined _WIN64 || defined _WIN32

#include <windows.h>

#include "tools/eclog.h"
#include "types/ecstream.h"

#define BUFSIZE 4096

typedef struct
{

  HANDLE hr;
  HANDLE hw;

  //OVERLAPPED stOverlapped;

  char buffer[BUFSIZE];

  DWORD dwState;

  EcStream stream;

  //int initial;

} EcPipe;

struct EcExec_s
{

  char* arguments[11];
    
  EcPipe errPipe;
  EcPipe outPipe;

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

  self->errPipe.hr = NULL;
  self->errPipe.hw = NULL;
  self->outPipe.hr = NULL;
  self->outPipe.hw = NULL;

  self->outPipe.stream = ecstream_create ();
  self->errPipe.stream = ecstream_create ();
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecexec_delete (EcExec* pself)
{
  int i;
  EcExec self = *pself;

  for (i = 1; i < 10; i++) {
    ecstr_delete(&(self->arguments[i]));
  }
  
  ENTC_DEL (pself, struct EcExec_s);
}

//-----------------------------------------------------------------------------------

void ecexec_addParameter (EcExec self, const EcString parameter)
{
  int i;
  for (i = 1; i < 10; i++) {
    if (isNotAssigned(self->arguments[i])) {
      self->arguments[i] = ecstr_copy(parameter);
      self->arguments[i + 1] = NULL;
      return;
    }
  }
}

//-----------------------------------------------------------------------------------

void ecexec_loop (EcExec self, HANDLE processh)
{
  DWORD dwWait;
  DWORD i;
  
  while (1) 
  {
	DWORD fSuccess, cbRet;
    HANDLE hds[2];
	EcPipe* pipes[2];
	EcPipe* pipe;

	DWORD c = 0;

	/*
	{
	  hds[0] = processh;
	  pipes[0] = NULL;
	  c++;
	}
	*/

	if (self->outPipe.hr != NULL)
	{
	  hds[c] = self->outPipe.hr;
	  pipes[c] = &(self->outPipe);
	  c++;
	}

	if (self->errPipe.hr != NULL)
	{
	  hds[c] = self->errPipe.hr;
	  pipes[c] = &(self->errPipe);
	  c++;
	}

	eclog_fmt (LL_TRACE, "ENTC", "exec", "wait for %i handles", c);

	dwWait = WaitForMultipleObjects(c, hds, FALSE, INFINITE);    // waits indefinitely 
    i = dwWait - WAIT_OBJECT_0;

	pipe = pipes[i];

	if (pipe != NULL) {
      fSuccess = ReadFile(pipe->hr, pipe->buffer, BUFSIZE, &cbRet, NULL);
	  if (!fSuccess) 
      {
	    eclog_err_os (LL_ERROR, "ENTC", "exec", "read IO failure");
		CloseHandle (pipe->hr);
		pipe->hr = NULL;
		if (c < 2) {
	      return;
	    }
		continue;
	  }

	  if (cbRet == 0)
	  {
	    eclog_err_os (LL_ERROR, "ENTC", "exec", "no data for read");
	    return;
	  }

  	  eclog_fmt (LL_TRACE, "ENTC", "exec", "recv data: '%i'", cbRet);

	  pipe->buffer[cbRet] = 0;

	  eclog_fmt (LL_TRACE, "ENTC", "exec", "recv '%s'", pipe->buffer);

	  ecstream_append_buf (pipe->stream, pipe->buffer, cbRet);

	}

  }
}

//-----------------------------------------------------------------------------------

void ecexec_createChildProcess (EcExec self)
{
  STARTUPINFO siStartInfo;
  PROCESS_INFORMATION piProcInfo;
  BOOL bSuccess = FALSE;
  EcString commandline;

  // Set up members of the PROCESS_INFORMATION structure. 
  ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
  // Set up members of the STARTUPINFO structure. 
  // This structure specifies the STDIN and STDOUT handles for redirection.
  ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
  siStartInfo.cb = sizeof(STARTUPINFO); 
  siStartInfo.hStdError = self->errPipe.hw;
  siStartInfo.hStdOutput = self->outPipe.hw;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
  
  commandline = ecstr_cat4 ("cmd.exe /C ", self->arguments[0], " ", self->arguments[1]);

  eclog_fmt (LL_TRACE, "ENTC", "exec:run", "execute: '%s'", commandline);

  bSuccess = CreateProcess(NULL,
	  commandline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 

  if (bSuccess) 
  {
	CloseHandle(self->outPipe.hw);
	CloseHandle(self->errPipe.hw);

	ecexec_loop (self, piProcInfo.hProcess);

	CloseHandle (piProcInfo.hProcess);
	CloseHandle (piProcInfo.hThread);
  }
  else
  {
	eclog_err_os (LL_TRACE, "ENTC", "exec:run", "failed to execute");
  }
}

//-----------------------------------------------------------------------------------

int ecexec_run (EcExec self)
{
  SECURITY_ATTRIBUTES saAttr;

  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
  saAttr.bInheritHandle = TRUE; 
  saAttr.lpSecurityDescriptor = NULL; 

  // Create a pipe for the child process's STDOUT.
  if ( ! CreatePipe(&(self->outPipe.hr), &(self->outPipe.hw), &saAttr, 0) ) 
  {
	eclog_msg (LL_ERROR, "ENTC", "exec", "can't create stdout pipe");
	return 0;
  }

  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if ( ! SetHandleInformation(self->outPipe.hr, HANDLE_FLAG_INHERIT, 0) )
  {
	eclog_msg (LL_ERROR, "ENTC", "exec", "set handle information for stdout failed");
	return 0;
  }

  // Create a pipe for the child process's STDOUT.
  if ( ! CreatePipe(&(self->errPipe.hr), &(self->errPipe.hw), &saAttr, 0) ) 
  {
	eclog_msg (LL_ERROR, "ENTC", "exec", "can't create stderr pipe");
	return 0;
  }

  // Ensure the read handle to the pipe for STDOUT is not inherited.
  if ( ! SetHandleInformation(self->errPipe.hr, HANDLE_FLAG_INHERIT, 0) )
  {
	eclog_msg (LL_ERROR, "ENTC", "exec", "set handle information for stderr failed");
	return 0;
  }

  // Create the child process.  
  ecexec_createChildProcess (self);

  return 0;
}

//-----------------------------------------------------------------------------------

const EcString ecexec_stdout (EcExec self)
{
  return ecstream_get (self->outPipe.stream);
}

//-----------------------------------------------------------------------------------

const EcString ecexec_stderr (EcExec self)
{
  return ecstream_get (self->errPipe.stream);
}

//-----------------------------------------------------------------------------------

#endif
