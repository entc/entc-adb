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

#include "qcsocket.h"
#include "../qclogger.h"

#include "qcevents_intern.h"

#include <stdio.h>


// include ucip1 tcp/ip stack
#include <qctcp.h>
#include <qsecfile.h>

/*------------------------------------------------------------------------*/

struct QCSocket_s
{

  QCEventContext eventcontext;
  
  void* handle;
  
};

//-----------------------------------------------------------------------------------

QCSocket qcsocket_new(QCEventContext eventcontext, QCLogger logger)
{
  QCSocket self = QNEW(struct QCSocket_s);
  
  self->eventcontext = eventcontext;
  self->handle = qctcp_new();
  
  return self;
}

//-----------------------------------------------------------------------------------

void qcsocket_delete(QCSocket* pself)
{
  QCSocket self = *pself;

  if (self->handle != NULL)
  {
    qctcp_delete(&(self->handle));
  }

  QDEL(pself, struct QCSocket_s);
}

//-----------------------------------------------------------------------------------

int qcsocket_connect(QCSocket self, const QCString host, uint_t port)
{
  
}

//-----------------------------------------------------------------------------------

int qcsocket_listen(QCSocket self, const QCString host, uint_t port)
{
  int res = qctcp_listen(self->handle, port);

  if (res == TRUE)
  {
    qclogger_logformat(self->logger, LOGMSG_DEBUG, "CORE", "Listen on %s:%u", host, port);
  }

  return res;
}

//-----------------------------------------------------------------------------------

QCSocket qcsocket_accept(QCSocket self)
{
  void* nhandle;
  QCSocket nsock = 0;

  qclogger_log(self->logger, LOGMSG_DEBUG, "CORE", "Waiting for incoming socket");

#ifdef __WITH_RTOS__
  while (qceventcontext_isTriggered(self->eventcontext) == FALSE) {
#else
  while (1) {
#endif
    // do some break
    
    int res = qctcp_poll_accept(self->handle, &nhandle);
    
    if (res == FALSE) 
    {
      continue;
    }
    
    nsock = QNEW(struct QCSocket_s);    
    nsock->handle = nhandle;
    break;
  }
  
  qclogger_logformat(self->logger, LOGMSG_DEBUG, "CORE", "Accept connection on socket <- %p", nsock);
  
  return nsock;
}

//-----------------------------------------------------------------------------------

int qcsocket_read(QCSocket self, void* buffer, int nbyte)
{
  // wait maximal 500 milliseconds
  return qcsocket_readTimeout(self, buffer, nbyte, 500);  
}

//-----------------------------------------------------------------------------------

int qcsocket_readTimeout(QCSocket self, void* buffer, int nbyte, int sec)
{
  int diff;
#ifdef __WITH_RTOS__
  while (qceventcontext_isTriggered(self->eventcontext) == FALSE) {
#else
    
  signed long start_time = qctcp_ticks();
    
  while (1) {
#endif

    int res = qctcp_poll_recv(self->handle, buffer, nbyte);

    if (res > 0) 
    {
      return res;
    }
    
    diff = qctcp_ticks() - start_time;    
    if (diff > 9) // 500 ms 
    {
      qclogger_log(self->logger, LOGMSG_WARNING, "CORE", "Timeout on socket");
      return 0;
    }
    
    
  }
}

//-----------------------------------------------------------------------------------

int qcsocket_write(QCSocket self, const void* buffer, int nbyte)
{
  return qctcp_send(self->handle, buffer, nbyte);
}

//-----------------------------------------------------------------------------------

int qcsocket_writeStream(QCSocket self, QCStream stream)
{
  uint_t size = qcstream_size( stream );
  
  return qcsocket_write(self, qcstream_buffer(stream), size);   
}
  
//-----------------------------------------------------------------------------------

int qcsocket_writeFile(QCSocket self, QCFileHandle fh)
{
  QCBuffer buffer = qcstr_buffer(1024);
    
  uint_t res = qcfh_readBuffer(fh, buffer);
    
  while( res )
  {
    qcsocket_write(self, buffer->buffer, res);
    res = qcfh_readBuffer(fh, buffer);
  }
  return TRUE;
}

//-----------------------------------------------------------------------------------

const QCString qcsocket_address(QCSocket self)
{
  return 0;
}

//-----------------------------------------------------------------------------------

#endif
