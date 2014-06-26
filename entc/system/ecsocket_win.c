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

#if defined _WIN64 || defined _WIN32

/* includes */
#include <ws2tcpip.h>
#include <winsock2.h>

#include "ecsocket.h"

#include <stdio.h>
#include <windows.h>

/* other core includes */
#include "ecmutex.h"
#include "ecevents.h"

#pragma comment(lib, "Ws2_32.lib")
/* static initialisation of win sockets */

struct EcSocket_s
{
  
  EcEventContext ec;
  
  EcLogger logger;
  
  SOCKET socket;
  
  EcMutex mutex;
  
};

//-----------------------------------------------------------------------------------

static WSADATA wsa;

int ecwsa_init()
{
  static int init = FALSE;
  
  if( init )
  {
    return TRUE;
  }
  else
  {
    if( (WSAStartup(MAKEWORD(2,2), &wsa)) == 0 )
    {
      init = TRUE;
    }
    
    return init;
  }
};

//-----------------------------------------------------------------------------------

EcSocket ecsocket_new(EcEventContext ec, EcLogger logger)
{
  EcSocket self = ENTC_NEW(struct EcSocket_s);
  
  ecwsa_init();

  self->ec = ec;
  self->logger = logger;
  self->socket = INVALID_SOCKET;
  self->mutex = NULL;
  
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
  
  if (self->socket != INVALID_SOCKET)
  {
    shutdown(self->socket, SD_BOTH);
    
    closesocket(self->socket);
    
    self->socket = INVALID_SOCKET;
  }
  
  ENTC_DEL(pself, struct EcSocket_s);
}

//-----------------------------------------------------------------------------------

SOCKET ecsocket_create(EcSocket self, const EcString host, uint_t port, int role)
{
  struct addrinfo hints;
  struct addrinfo* addr;
  SOCKET sock;
  EcString port_s;
  
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = role ? 0 : AI_PASSIVE;
  
  port_s = ecstr_long (port);

  if (getaddrinfo(host, port_s, &hints, &addr) != 0) 
  {
    return INVALID_SOCKET;
  }

  ecstr_delete(&port_s);

  sock = socket (addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  
  if (sock == INVALID_SOCKET)
  {
    freeaddrinfo(addr);
    return INVALID_SOCKET;
  }
  
  if (role) 
  {
    if (connect(sock, addr->ai_addr, (int)addr->ai_addrlen) == SOCKET_ERROR) 
    {
      freeaddrinfo(addr);
      closesocket(sock);
      return INVALID_SOCKET;
    }
  } 
  else 
  {
    if (bind(sock, addr->ai_addr, (int)addr->ai_addrlen) == SOCKET_ERROR) 
    {
      freeaddrinfo(addr);
      closesocket(sock);
      return INVALID_SOCKET;
    }
  }
  
  freeaddrinfo(addr);
  return sock;
}

//-----------------------------------------------------------------------------------

int ecsocket_connect(EcSocket self, const EcString host, uint_t port)
{
  self->socket = ecsocket_create(self, host, port, TRUE);
  
  return self->socket != INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------

int ecsocket_listen(EcSocket self, const EcString host, uint_t port)
{
  SOCKET sock = ecsocket_create(self, host, port, FALSE);
  
  if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
  {
    closesocket(sock);
    return FALSE;
  }
  
  self->socket = sock;
  // indicates a listen socket
  self->mutex = ecmutex_new();
  
  return TRUE;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_accept (EcSocket self)
{
  // variables
  EcEventQueue queue;
  HANDLE shandle;
  SOCKET sock = INVALID_SOCKET;
  EcSocket nself;

  if (self->mutex == NULL)
  {
    return NULL;
  }
  // critical section start
  ecmutex_lock(self->mutex);
  
  shandle = WSACreateEvent();
  
  if( WSAEventSelect( self->socket, shandle, FD_ACCEPT | FD_CLOSE ) == SOCKET_ERROR )
  {
    ecmutex_unlock(self->mutex);
    return 0;
  }
  
  queue = ece_queue_new(self->ec);
  ece_queue_add (queue, shandle, ENTC_EVENTTYPE_READ);

  while (sock == INVALID_SOCKET) 
  {
    int res = ece_queue_wait (queue, ENTC_INFINTE, self->logger);
    // check the return
    if (res == 1)
    {
      // termination of the process
      break;
    }
    if (res == 0)
    {
      // timeout
      break;
    }
    // try to accept connection
    sock = accept(self->socket, 0, 0 );
    
    if (sock == INVALID_SOCKET)
    {
      int lasterr = WSAGetLastError();
      // Ingnore this error
      if (lasterr == WSAEWOULDBLOCK) 
      {
        continue;
      }
      break;
    }      
  }
  // delete event list
  ece_queue_delete (&queue);

  WSACloseEvent(shandle);
  
  // leave critical section
  ecmutex_unlock(self->mutex);
  
  if (sock == INVALID_SOCKET) 
  {
    return NULL;
  }
  
  nself = ENTC_NEW(struct EcSocket_s);
  
  nself->ec = self->ec;
  nself->logger = self->logger;
  nself->socket = sock;
  nself->mutex = ecmutex_new();
  
  return nself;  
}

//-----------------------------------------------------------------------------------

int ecsocket_read(EcSocket self, void* buffer, int nbyte)
{
  // wait maximal 500 milliseconds
  return ecsocket_readTimeout(self, buffer, nbyte, 500);
}

//-----------------------------------------------------------------------------------

int ecsocket_readTimeout(EcSocket self, void* buffer, int nbyte, int timeout)
{
  // variables
  EcEventQueue queue;
  HANDLE shandle;
  int ret;
  
  if (self->mutex == NULL)
  {
    return ENTC_SOCKET_RETSTATE_ERROR;
  }
  // critical section start
  ecmutex_lock(self->mutex);
  
  shandle = WSACreateEvent();
  
  if( WSAEventSelect( self->socket, shandle, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
  {
    ecmutex_unlock(self->mutex);
    return ENTC_SOCKET_RETSTATE_ERROR;
  }
  
  queue = ece_queue_new (self->ec);
  ece_queue_add (queue, shandle, ENTC_EVENTTYPE_READ);
  
  while (TRUE) 
  {
    int res = ece_queue_wait (queue, ENTC_INFINTE, self->logger);
    // check the return
    if (res == 1)
    {
      // termination of the process
      ret = ENTC_SOCKET_RETSTATE_ABORT;
      break;
    }
    if (res == 0)
    {
      // timeout
      ret = ENTC_SOCKET_RETSTATE_ERROR;
      break;
    }
    
    ret = recv(self->socket, buffer, nbyte, 0);  
    if (ret == SOCKET_ERROR)
    {
      int lasterr = WSAGetLastError();
      // Ingnore this error
      if (lasterr == WSAEWOULDBLOCK) 
      {
        continue;
      }
    }
    // end
    break;
  }  
  // delete event list
  ece_queue_delete (&queue);
  
  WSACloseEvent(shandle);
  
  // leave critical section
  ecmutex_unlock(self->mutex);

  return ret;  
}

//-----------------------------------------------------------------------------------

int ecsocket_write(EcSocket self, const void* buffer, int nbyte)
{
  // variables
  int ret = 0;
  int del = 0;
  
  if (self->mutex == NULL)
  {
    return ENTC_SOCKET_RETSTATE_ERROR;
  }
  // critical section start
  
  while (del < nbyte)
  {
    // send
    ret = send(self->socket, (char*)buffer + del, nbyte - del, 0);
    if (ret == 0)
    {
      break;
    }
    else if (ret == SOCKET_ERROR)
    {
      int lasterr = WSAGetLastError();
      // Ingnore this error
      if (lasterr == WSAEWOULDBLOCK) 
      {
        continue;
      }
      break;
    }
    else
    {
      del = del + ret;
    }
  }  
  return ret;  
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
  
  return TRUE;
}

//-----------------------------------------------------------------------------------

int ecsocket_writeStream(EcSocket self, EcStream stream)
{
  uint_t size = ecstream_size( stream );
  
  return ecsocket_write(self, ecstream_buffer(stream), size);  
}

//-----------------------------------------------------------------------------------

const EcString ecsocket_address(EcSocket self)
{
  return 0;
}

//-----------------------------------------------------------------------------------

#endif
