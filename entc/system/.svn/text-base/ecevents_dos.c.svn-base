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

#if defined __DOS__

#include "qcevents.h"
#include "qcevents_intern.h"

#include <queue.h>
#include <task.h>

#include <stdio.h>

typedef struct
{

  int dummy;

} EcEventList;

struct EcEventContext_s
{
  
  xQueueHandle xQueue;
  
};

/*------------------------------------------------------------------------*/

const unsigned portBASE_TYPE uxQueueSize1 = 1, uxQueueSize5 = 5;

/*------------------------------------------------------------------------*/

EcEventContext eceventcontext_new(void)
{
  EcEventContext self = QNEW(struct EcEventContext_s);

  taskENTER_CRITICAL();
  
  self->xQueue = xQueueCreate( 10, ( unsigned portBASE_TYPE ) sizeof( unsigned short ) );
  
  vQueueAddToRegistry( self->xQueue, ( signed char * ) "Queue" );
  
  taskEXIT_CRITICAL();
  
  return self;
}

/*------------------------------------------------------------------------*/

void eceventcontext_delete(EcEventContext* ptr)
{
  EcEventContext self = *ptr;
  
  taskENTER_CRITICAL();
  
  vQueueDelete(self->xQueue);
  
  taskEXIT_CRITICAL();
  
  QDEL(ptr, struct EcEventContext_s);
}

/*------------------------------------------------------------------------*/

void eceventcontext_abort(EcEventContext self)
{
  unsigned short usValue = 0;
  
  if( xQueueSend( self->xQueue, ( void * ) &usValue, 0 ) != pdPASS )
  {
    printf("abort failed\n");
    return;
  }
}

/*------------------------------------------------------------------------*/

int eceventcontext_wait(EcEventContext self, uint_t timeout, EcLogger logger)
{
  unsigned short usData = 0;
    
  portTickType ticktime = ( portTickType ) timeout / portTICK_RATE_MS;
  
  if (timeout == Ec_INFINTE) 
  {
    for(;;) 
    {
      if (xQueueReceive( self->xQueue, &usData, ticktime ) == pdPASS) 
      {
        return TRUE;
      }
    }
  } 
  else 
  {    
    return xQueueReceive( self->xQueue, &usData, ticktime ) == pdPASS; 
  }
}

/*------------------------------------------------------------------------*/

int eceventcontext_isTriggered(EcEventContext self)
{
  unsigned short usData = 0;

  if (self == NULL)
  {
    return FALSE;
  }
  
  if (xQueueReceive( self->xQueue, &usData, 0 ) == pdPASS) 
  {
    eceventcontext_abort(self);
    
    return TRUE;
  }
  
  return FALSE;
}
  
/*------------------------------------------------------------------------*/

typedef struct
{
    
  int dummy;
  
} EcEventsData;

struct EcEvents_s
{
    
  int dummy;
  
};

/*------------------------------------------------------------------------*/
 
EcEvents ecevents_new(EcLogger logger)
{
  return NULL;
}

/*------------------------------------------------------------------------*/
 
void ecevents_delete(EcEvents* ptr)
{
}

/*------------------------------------------------------------------------*/

void ecevents_register(EcEvents self, const EcString filename, events_callback_fct onChange, void* onChangePtr, events_callback_fct onDelete, void* onDeletePtr)
{
}

/*------------------------------------------------------------------------*/

#endif
