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

#ifdef __DOS__

#include "qcsignal.h"

#include <task.h>
#include <queue.h>
#include <stdio.h>

#ifdef __WITH_RTOS__

static QCEventContext globalEventContext = NULL;
static xTaskHandle xHandle;

/*------------------------------------------------------------------------*/

static void qcsignal_run( void *pvParameters )
{  
  for(;;) 
  {
    taskENTER_CRITICAL();
    {
      short sIn = getch();
      if (sIn == 3)
      {
        qceventcontext_abort(globalEventContext);
      }
    }
    taskEXIT_CRITICAL();
  }  
}

/*------------------------------------------------------------------------*/

QCEventContext qcsignal_init(void)
{
  globalEventContext = qceventcontext_new();
  
  taskENTER_CRITICAL();
  
  xTaskCreate( qcsignal_run, "signal", (unsigned short)512, NULL, tskIDLE_PRIORITY + 4, &xHandle );
  
  taskEXIT_CRITICAL();

  return globalEventContext;
}

/*------------------------------------------------------------------------*/

void qcsignal_done(void)
{
  taskENTER_CRITICAL();
  
  vTaskDelete(xHandle);
  
  taskEXIT_CRITICAL();
  
  qceventcontext_delete(&globalEventContext);
}

/*------------------------------------------------------------------------*/

#else

volatile uint8_t CtrlBreakDetected = 0;

#if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )

void ( __interrupt __far *oldCtrlBreakHandler)( );

void __interrupt __far ctrlBreakHandler( ) {
  CtrlBreakDetected = 1;
}
#else

#endif


/*------------------------------------------------------------------------*/

QCEventContext qcsignal_init(void)
{
  oldCtrlBreakHandler = getvect( 0x1b );

  setvect( 0x1b, ctrlBreakHandler);  // Ctrl-Break handler
  setvect( 0x23, ctrlBreakHandler);  // Ctrl-C handler  
}

/*------------------------------------------------------------------------*/

void qcsignal_done(void)
{
}

/*------------------------------------------------------------------------*/

#endif

#endif
