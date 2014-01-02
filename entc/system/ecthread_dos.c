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

#include "qcthread.h"
#include "qcmutex.h"
#include "qcevents.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <croutine.h>
#include <stdio.h>


static xQueueHandle xPrintQueue;

//-----------------------------------------------------------------------------------

/*
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
  printf("good \n");
}

void vApplicationTickHook( void )
{
  printf("tick \n");  
}
*/

void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
}

//-----------------------------------------------------------------------------------

struct EcThread_s {
  
  ecthread_callback_fct fct;
  
  void* ptr;
  
  xTaskHandle handle;
      
};

//-----------------------------------------------------------------------------------

typedef struct {
  
  ecthread_main_fct fct;
  
  int argc;
  
  char* *argv;
  
} EcThreadParams;

//-----------------------------------------------------------------------------------

static void ecthread_run( void *pvParameters )
{
  EcThread self = pvParameters;
  
  printf("entered thread %p\n", self);
  
  if (self->fct)
  {
    while (self->fct(self->ptr));
  }
}

//-----------------------------------------------------------------------------------

static void ecthread_main( void *pvParameters )
{
  EcThreadParams* params = pvParameters;
  
  if (params->fct) 
  {
    params->fct(params->argc, params->argv);
  }
  
  vTaskEndScheduler();
  
  ENTC_DEL(&params, EcThreadParams);
}

//-----------------------------------------------------------------------------------

void ecthread_schedule(ecthread_main_fct main, int argc, char* argv[])
{
  const unsigned portBASE_TYPE uxQueueSize = 20;
  
  EcThreadParams* params = ENTC_NEW(EcThreadParams);
  
  
  xPrintQueue = xQueueCreate( uxQueueSize, ( unsigned portBASE_TYPE ) sizeof( char * ) );

  
  params->fct = main;
  params->argc = argc;
  params->argv = argv;
  xTaskCreate( ecthread_main, (signed char* const)"main", (unsigned short)255, params, tskIDLE_PRIORITY + 3, NULL );
  
  vTaskStartScheduler();  
}

//-----------------------------------------------------------------------------------

EcThread ecthread_new(void)
{
  EcThread self = ENTC_NEW(struct EcThread_s);
  
  self->fct = NULL;
  self->ptr = NULL;
  self->handle = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecthread_delete(EcThread* pself)
{
 // EcThread self = *pself;
  
  
  ENTC_DEL(pself, struct EcThread_s);
}

//-----------------------------------------------------------------------------------

void ecthread_start(EcThread self, ecthread_callback_fct fct, void* ptr)
{
  self->fct = fct;
  self->ptr = ptr;
  
  taskENTER_CRITICAL();
  
  xTaskCreate( ecthread_run, (signed char* const)"th", (unsigned short)255, self, tskIDLE_PRIORITY + 4, &(self->handle) );  
  
  taskEXIT_CRITICAL();
}

//-----------------------------------------------------------------------------------

void ecthread_stop(EcThread self)
{

}

//-----------------------------------------------------------------------------------

#endif
