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

#include "ecevents.h"
#include "ecsocket.h"
#include "../utils/ecreadbuffer.h"

/* other core includes */
#include "ecmutex.h"

#include <sys/types.h>	/* basic system data types */

/* includes */
#include <sys/socket.h>	/* basic socket definitions */
#include <arpa/inet.h>	/* inet(3) functions */
#include <stdio.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

//-----------------------------------------------------------------------------------

struct EcSocket_s
{
  
  EcEventContext ec;

  EcLogger logger;
  
  int socket;
    
  EcMutex mutex;
  
  EcString host;
  
};

//-----------------------------------------------------------------------------------

EcSocket ecsocket_new(EcEventContext ec, EcLogger logger)
{
  EcSocket self = ENTC_NEW(struct EcSocket_s);
  
  self->ec = ec;
  self->logger = logger;
  self->socket = 0;
  self->mutex = NULL;
  self->host = ecstr_init();
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecsocket_delete(EcSocket* pself)
{
  EcSocket self = *pself;
  
  if (self->mutex)
  {
    ecmutex_delete(&(self->mutex));
  }
  
  if (self->socket >= 0)
  {
    shutdown(self->socket, 2);
    
    close(self->socket);
    
    self->socket = -1;
  }
  
  ecstr_delete(&(self->host));
  
  ENTC_DEL(pself, struct EcSocket_s);  
}

//-----------------------------------------------------------------------------------

int ecsocket_create(EcSocket self, const EcString host, uint_t port, int role)
{
  struct sockaddr_in addr;
  struct hostent* server;
  int sock;
  
  memset ( &addr, 0, sizeof(addr) );
  /* set the address */
  addr.sin_family = AF_INET;
  /* set the port */
  addr.sin_port = htons(port);
  /* set the address */
  if (ecstr_empty(host))
  {
    addr.sin_addr.s_addr = INADDR_ANY;    
  }
  else
  {
    server = gethostbyname(host);
    
    if(server)
    {
      bcopy((char*)server->h_addr, (char*)&(addr.sin_addr.s_addr), server->h_length);
    }
    else
    {
      addr.sin_addr.s_addr = INADDR_ANY;      
    }
  }
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Can't create socket");
    return -1;
  }
  
  if (role) 
  {
    if (connect(sock, (const struct sockaddr*)&(addr), sizeof(addr)) < 0) 
    {
      close(sock);      
      eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Can't connect to '%s:%u'", host, port);
      return -1;
    }
  }
  else
  {
    int opt = 1; 
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
      eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Can't set socket option");
      close(sock);
      return -1;
    }    
    
    if (bind(sock, (const struct sockaddr*)&(addr), sizeof(addr)) < 0) 
    {
      close(sock);
      eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Can't bind to '%s:%u'", host, port);
      return -1;
    }
  }
  
  return sock;
}

//-----------------------------------------------------------------------------------

int ecsocket_connect(EcSocket self, const EcString host, uint_t port)
{
  self->socket = ecsocket_create(self, host, port, TRUE); 
  
  if (self->socket >= 0)
  {
    self->mutex = ecmutex_new();
    
    ecstr_replace(&(self->host), host);
    return TRUE;
  }
  else
  {
    ecstr_delete(&(self->host));
    return FALSE;
  }
}

//-----------------------------------------------------------------------------------

int ecsocket_listen(EcSocket self, const EcString host, uint_t port)
{
  int sock = ecsocket_create(self, host, port, FALSE);
  if (sock < 0) 
  {
    return FALSE;
  }
  // cannot fail
  listen(sock, SOMAXCONN);
  
  self->socket = sock;
  // indicates a listen socket
  self->mutex = ecmutex_new(); 
  ecstr_replace(&(self->host), host);
  
  return TRUE;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_accept (EcSocket self)
{
  // variables
  EcEventQueue queue;
  int sock = -1;
  EcSocket nself = NULL;
    
  struct sockaddr addr;
  socklen_t addrlen = 0;
  
  memset ( &addr, 0, sizeof(addr) );
  
  if (self->mutex == NULL)
  {
    return NULL;
  }
  // critical section start
  ecmutex_lock (self->mutex);
  
  queue = ece_queue_new (self->ec);  
  ece_queue_add (queue, self->socket, ENTC_EVENTTYPE_READ);
  
  while (sock == -1) 
  {
    // wait until some data received on one of the handles
    int res = ece_queue_wait (queue, ENTC_INFINTE, self->logger);
    // check the return
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      break;
    }
    if (res == ENTC_EVENT_TIMEOUT)
    {
      // timeout
      break;
    }
    eclogger_log(self->logger, LL_TRACE, "CORE", "{socket} accept got data"); 
    // try to accept connection
    sock = accept(self->socket, &addr, &addrlen );
    if (sock < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Error on accept client connection"); 
        break;
      }
      else
      {
        eclogger_logerrno(self->logger, LL_TRACE, "CORE", "Minor rrror on accept client connection"); 
        continue;
      }
    }
  }
  // delete event list
  ece_queue_delete (&queue);  
  // leave critical section
  ecmutex_unlock(self->mutex);
  
  if (sock < 0) 
  {
    return NULL;
  }
  
  nself = ENTC_NEW(struct EcSocket_s);
  
  nself->ec = self->ec;
  nself->logger = self->logger;
  nself->socket = sock;
  nself->mutex = ecmutex_new();
  
  {
    EcBuffer ipbuffer = ecbuf_create (INET6_ADDRSTRLEN);    
    // convert address information into string
    inet_ntop(AF_INET, &addr, (char*)ipbuffer->buffer, addrlen);
    
    nself->host = ecbuf_str (&ipbuffer);
    
    eclogger_logformat(self->logger, LL_TRACE, "CORE", "{socket} connection accepted from '%s'", nself->host);
  }
  
  return nself;   
}

//-----------------------------------------------------------------------------------

int ecsocket_read (EcSocket self, void* buffer, int nbyte)
{
  int bytesread = 0;
  while (bytesread < nbyte)
  {
    int h = ecsocket_readTimeout (self, (unsigned char*)buffer + bytesread, nbyte, 30000);
    if (h > 0)
    {
      bytesread += h;
    }
    else
    {
      return h;
    }
  }
  return nbyte;  
}

//-----------------------------------------------------------------------------------

int ecsocket_readBunch (EcSocket self, void* buffer, int nbyte)
{
  // wait maximal 30 seconds
  return ecsocket_readTimeout(self, buffer, nbyte, 30000);    
}

//-----------------------------------------------------------------------------------

int ecsocket_readTimeout (EcSocket self, void* buffer, int nbyte, int timeout)
{
  // variables
  EcEventQueue queue;
  int ret;
  
  if (self->mutex == NULL)
  {
    return ENTC_SOCKET_RETSTATE_ERROR;
  }
  // critical section start
  ecmutex_lock(self->mutex);

  queue = ece_queue_new (self->ec);  
  ece_queue_add (queue, self->socket, ENTC_EVENTTYPE_READ);
  
  while (TRUE) 
  {
    int res = ece_queue_wait (queue, timeout, self->logger);
    // check the return
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      ret = ENTC_SOCKET_RETSTATE_ABORT;
      break;
    }
    if (res == ENTC_EVENT_TIMEOUT)
    {
      // timeout
      ret = ENTC_SOCKET_RETSTATE_TIMEOUT;
      break;
    }
    
    ret = recv(self->socket, buffer, nbyte, 0);  
    if (ret < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Error on recv"); 
        break;
      }
      else
      {
        continue;
      }
    }
    else if (ret == 0)
    {
      eclogger_logerrno(self->logger, LL_WARN, "CORE", "{socket} connection reset by host"); 
      ret = ENTC_SOCKET_RETSTATE_ERROR;
    }    
    // end
    break;
  }  
  // delete event list
  ece_queue_delete (&queue);  
  
  // leave critical section
  ecmutex_unlock(self->mutex);
  
  return ret;    
}

//-----------------------------------------------------------------------------------

int ecsocket_write (EcSocket self, const void* buffer, int nbyte)
{
  // variables
  EcEventQueue queue;
  int ret = 0;
  int del = 0;
  
  if (self->mutex == NULL)
  {
    return ENTC_SOCKET_RETSTATE_ERROR;
  }
  // critical section start
  //ecmutex_lock(self->mutex);
  
  queue = ece_queue_new (self->ec);  
  ece_queue_add (queue, self->socket, ENTC_EVENTTYPE_WRITE);
  
  while (del < nbyte)
  {
    // wait maximum 10 seconds
    int res = ece_queue_wait (queue, ENTC_INFINTE, self->logger);
    // check the return
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      ret = ENTC_SOCKET_RETSTATE_ABORT;
      break;
    }
    if (res == ENTC_EVENT_TIMEOUT)
    {
      // timeout
      ret = ENTC_SOCKET_RETSTATE_ERROR;
      break;
    }
    // send
    ret = send(self->socket, (char*)buffer + del, nbyte - del, 0);
    if (ret < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_logerrno(self->logger, LL_ERROR, "CORE", "Error on recv"); 
        break;
      }
      else
      {
        continue;
      }
    }
    else if (ret == 0)
    {
      ret = ENTC_SOCKET_RETSTATE_ERROR;
      break;
    }
    else
    {
      del = del + ret;
    }
  }
  // delete event list
  ece_queue_delete (&queue);   
  // leave critical section
  //ecmutex_unlock(self->mutex);
  
  return ret;    
}

//-----------------------------------------------------------------------------------

int ecsocket_writeStream(EcSocket self, EcStream stream)
{
  uint_t size = ecstream_size( stream );
  
  return ecsocket_write(self, ecstream_buffer(stream), size);  
}

//-----------------------------------------------------------------------------------

int ecsocket_writeFile(EcSocket self, EcFileHandle fh)
{
  // write raw data
  EcBuffer buffer = ecbuf_create (1024);
  
  uint_t res = ecfh_readBuffer(fh, buffer);
  
  while( res )
  {
    ecsocket_write(self, buffer->buffer, res);
    res = ecfh_readBuffer(fh, buffer);
  }
  
  ecbuf_destroy (&buffer);
  
  return TRUE;
}

//-----------------------------------------------------------------------------------

const EcString ecsocket_address(EcSocket self)
{
  return self->host;
}

//-----------------------------------------------------------------------------------

#endif
