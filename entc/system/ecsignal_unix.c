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

#ifdef __GNUC__

#include "ecsignal.h"

static EcEventContext globalEventContext = 0;

#include <signal.h>
#include <stdio.h>

void mySignalHandler(int signum)
{
  ece_context_setAbort (globalEventContext);
}

/*------------------------------------------------------------------------*/


void ecsignal_init (EcEventContext ec)
{
  globalEventContext = ec;
  
  /* redirect the signals */
  signal( SIGTERM, mySignalHandler );
  signal( SIGINT, mySignalHandler );
  
  {
    /* block signal broken pipe */
    sigset_t set, old;
    
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    
    pthread_sigmask(SIG_BLOCK, &set, &old);
  }
}

/*------------------------------------------------------------------------*/

void ecsignal_done (void)
{
  signal( SIGTERM, SIG_DFL );
  signal( SIGINT, SIG_DFL );
}

/*------------------------------------------------------------------------*/

#endif
