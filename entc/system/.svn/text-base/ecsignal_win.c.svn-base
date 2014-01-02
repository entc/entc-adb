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

#ifdef _WIN32

#include "ecsignal.h"

EcEventContext globalEventHandle;

#include <windows.h>
#include <signal.h>

static BOOL myCtrlHandler( DWORD fdwCtrlType ) 
{
  switch( fdwCtrlType ) 
  { 
    /* handle the CTRL-C signal */
    case CTRL_C_EVENT:
      ece_context_triggerTermination (globalEventHandle);
      Sleep(5000);

      return( TRUE );
      
    case CTRL_SHUTDOWN_EVENT: 
      ece_context_triggerTermination (globalEventHandle);
      Sleep(5000);

      return( TRUE );
      
    case CTRL_CLOSE_EVENT: 
      ece_context_triggerTermination (globalEventHandle);
      Sleep(5000);

      return( TRUE );

	default: 
      return FALSE; 
  }
}

/*------------------------------------------------------------------------*/


void ecsignal_init(EcEventContext ec)
{
  globalEventHandle = ec;
  /* create the control handlers */
  if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE)myCtrlHandler, TRUE ) ) 
  {
    //eclogger_logerrno(logger, LOGMSG_ERROR, "CORE", "{server} set ctrl handler" );
  }
}

/*------------------------------------------------------------------------*/

void ecsignal_done()
{  
  globalEventHandle = 0;
}

/*------------------------------------------------------------------------*/

#endif
