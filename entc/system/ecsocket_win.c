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
  // reference
  EcEventContext ec;
  // the original fd socket 
  SOCKET socket;
  // accept windows event handle
  HANDLE haccept;
  // read windows event handle
  HANDLE hread;

  int protocol;
  
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
}

//-----------------------------------------------------------------------------------

EcHandle ecsocket_createAcceptHandle (EcSocket self)
{
  HANDLE handle = WSACreateEvent();

  if (WSAEventSelect (self->socket, handle, FD_ACCEPT | FD_CLOSE ) == SOCKET_ERROR)
  {
    WSACloseEvent (handle);
    return NULL;
  }
  return handle;
}

//-----------------------------------------------------------------------------------

EcHandle ecsocket_createReadHandle (EcSocket self)
{
  HANDLE handle = WSACreateEvent();

  if (WSAEventSelect (self->socket, handle, FD_READ | FD_CLOSE ) == SOCKET_ERROR)
  {
    eclogger_errno (LL_ERROR, "ENTC", "socket", "can't create win handle");

    WSACloseEvent (handle);
    return NULL;
  }
  return handle;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_new (EcEventContext ec, int protocol)
{
  EcSocket self = ENTC_NEW(struct EcSocket_s);
  
  ecwsa_init();

  self->ec = ec;

  self->socket = INVALID_SOCKET;

  self->haccept = NULL;
  self->hread = NULL;

  self->protocol = protocol;
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecsocket_delete (EcSocket* pself)
{
  EcSocket self = *pself;
  
  if (self->haccept)
  {
    WSACloseEvent (self->haccept);  
  }
  if (self->hread)
  {
    WSACloseEvent (self->hread);  
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

SOCKET ecsocket_create (EcSocket self, const EcString host, uint_t port, int role)
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

  if (self->protocol == ENTC_SOCKET_PROTOCOL_UDP)
  {
    sock = socket (addr->ai_family, SOCK_DGRAM, IPPROTO_UDP);
  }
  else
  {
    sock = socket (addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  }  

  if (sock == INVALID_SOCKET)
  {
    eclogger_errno (LL_ERROR, "ENTC", "socket", "cna't create socket");

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

int ecsocket_connect (EcSocket self, const EcString host, uint_t port)
{
  self->socket = ecsocket_create(self, host, port, TRUE);
  
  return self->socket != INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------

int ecsocket_listen (EcSocket self, const EcString host, uint_t port)
{
  SOCKET sock = ecsocket_create(self, host, port, FALSE);
  
  if (self->protocol == ENTC_SOCKET_PROTOCOL_UDP)
  {
    self->socket = sock;

    self->haccept = ecsocket_createReadHandle (self);
    if (isNotAssigned (self->haccept))
    {
      closesocket(sock);
      return FALSE;
    }
    return TRUE;
  }

  if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
  {
    closesocket(sock);
    return FALSE;
  }
  
  self->socket = sock;
  // indicates a listen socket
  self->haccept = ecsocket_createAcceptHandle (self);
  if (isNotAssigned (self->haccept))
  {
    closesocket(sock);
    return FALSE;
  }
  
  return TRUE;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_createReadSocket (EcEventContext ec, SOCKET sock)
{
  EcSocket self = ENTC_NEW(struct EcSocket_s);
  // references
  self->ec = ec;
  // socket
  self->socket = sock;
  //handles
  self->haccept = NULL;
  self->hread = ecsocket_createReadHandle (self);
  if (isNotAssigned (self->hread))
  {
    ecsocket_delete (&self);
  }
  return self;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_accept (EcSocket self)
{
  while (TRUE) 
  {
    SOCKET sock = accept (self->socket, 0, 0); 
    if (sock == INVALID_SOCKET) 
    {
      int lasterr = WSAGetLastError();
      if (lasterr == WSAEWOULDBLOCK) 
      {
        Sleep (20);
        continue;
      }
      return NULL;
    }
    return ecsocket_createReadSocket (self->ec, sock);
  }  
}

//-----------------------------------------------------------------------------------

int ecsocket_readBunch (EcSocket self, void* buffer, int nbyte)
{
  while (TRUE) 
  {
    int res = recv (self->socket, buffer, nbyte, 0);  
    if (res == 0)
    {
      return 0;
    }
    else if (res == SOCKET_ERROR) 
    {
      int lasterr = WSAGetLastError();
      if (lasterr == WSAEWOULDBLOCK) 
      {
        Sleep (20);
        continue;
      }
      return -1;
    }
    return res;
  }  
}

//-----------------------------------------------------------------------------------
  
int ecsocket_write (EcSocket self, const void* buffer, int nbyte)
{
  int del = 0;
  while (del < nbyte)
  {
    int res = send (self->socket, (char*)buffer + del, nbyte - del, 0);
    if (res == 0)
    {
      return 0;
    } 
    else if (res == SOCKET_ERROR) 
    {
      int lasterr = WSAGetLastError();
      if (lasterr == WSAEWOULDBLOCK) 
      {
        Sleep (20);
        continue;
      }
      return -1;
    }
    else
    {
      del += res;
    }    
  }
  return nbyte;
}

//-----------------------------------------------------------------------------------

int ecsocket_writeFile (EcSocket self, EcFileHandle fh)
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

int ecsocket_writeStream (EcSocket self, EcStream stream)
{
  uint_t size = ecstream_size( stream );
  
  return ecsocket_write(self, ecstream_buffer(stream), size);  
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_acceptIntr (EcSocket self)
{ 
  while (TRUE) 
  {
    SOCKET sock;
    // wait for either data on the handle or terminate signal
    int res = ece_context_wait (self->ec, self->haccept, INFINITE, ENTC_EVENTTYPE_READ);
    if (res == ENTC_EVENT_ABORT)
    {
      // termination of the process
      return NULL;
    }
    if (res == ENTC_EVENT_TIMEOUT)
    {
      // timeout
      return NULL;
    }
    sock = accept (self->socket, 0, 0); 
    if (sock == INVALID_SOCKET) 
    {
      int lasterr = WSAGetLastError();
      if (lasterr == WSAEWOULDBLOCK) 
      {
        Sleep (20);
        continue;
      }
      // error
      return NULL;
    }
    return ecsocket_createReadSocket (self->ec, sock);
  }
}

//-----------------------------------------------------------------------------------

int ecsocket_readTimeout (EcSocket self, void* buffer, int nbyte, int timeout)
{
  while (TRUE) 
  {
    int res = ece_context_wait (self->ec, self->hread, timeout == ENTC_INFINITE ? INFINITE : timeout, ENTC_EVENTTYPE_READ);
    if (res == ENTC_EVENT_ABORT || res == ENTC_EVENT_TIMEOUT)
    {
      return res;
    }
    res = recv (self->socket, buffer, nbyte, 0); 
    if (res == 0)
    {
      return 0;
    } 
    else if (res == SOCKET_ERROR) 
    {
      int lasterr = WSAGetLastError();
      if (lasterr == WSAEWOULDBLOCK) 
      {
        Sleep (20);
        continue;
      }
      return -1;
    }
    return res;
  }
}

//-----------------------------------------------------------------------------------

int ecsocket_readIntr (EcSocket self, void* buffer, int nbyte, int timeout)
{
  int bytesread = 0;
  while (bytesread < nbyte)
  {
    int h = ecsocket_readTimeout (self, (unsigned char*)buffer + bytesread, nbyte, timeout);
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

int ecsocket_readIntrBunch (EcSocket self, void* buffer, int nbyte, int timeout)
{
  // wait maximal 500 milliseconds
  return ecsocket_readTimeout (self, buffer, nbyte, timeout);
}

//-----------------------------------------------------------------------------------

const EcString ecsocket_address (EcSocket self)
{
  return 0;
}

//-----------------------------------------------------------------------------------

EcHandle ecsocket_getAcceptHandle (EcSocket self)
{
  return self->haccept;
}

//-----------------------------------------------------------------------------------

EcHandle ecsocket_getReadHandle (EcSocket self)
{
  return self->hread;
}

//-----------------------------------------------------------------------------------

void ecsocket_resetHandle (EcHandle handle)
{
  ResetEvent (handle);
}

//-----------------------------------------------------------------------------------

struct EcDatagram_s
{
  
  EcBuffer buffer;
  
  EcSocket socket;
  
  struct sockaddr_in senderAddr;
  
  unsigned int senderAddrSize;
  
  EcBuffer ident;
  
};

//-----------------------------------------------------------------------------------

EcDatagram ecdatagram_create (EcSocket socket)
{
  EcDatagram self = ENTC_NEW (struct EcDatagram_s);
  
  self->buffer = ecbuf_create (1472);
  self->socket = socket;
  
  self->senderAddrSize = sizeof(struct sockaddr_in);
  self->ident = ecbuf_create (INET_ADDRSTRLEN + 6);
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecdatagram_destroy (EcDatagram* pself)
{
  EcDatagram self = *pself;
  
  ecbuf_destroy(&(self->buffer));
  ecbuf_destroy(&(self->ident));
  
  ENTC_DEL (pself, struct EcDatagram_s);
}

//-----------------------------------------------------------------------------------

size_t ecdatagram_read (EcDatagram self)
{
  size_t count;

  memset (&(self->senderAddr), 0, self->senderAddrSize);
  
  count = recvfrom (self->socket->socket, self->buffer->buffer, self->buffer->size, 0, (struct sockaddr*)&(self->senderAddr), &(self->senderAddrSize));
  
  if (count == -1)
  {
    eclogger_errno (LL_ERROR, "ENTC", "datagram", "Error on recv");
    return 0;
  }
  else if (count == self->buffer->size)
  {
    eclogger_errno (LL_WARN, "ENTC", "datagram", "datagram too large for buffer: truncated");
  }  

  // convert address information into string    
  inet_ntop (AF_INET, &(self->senderAddr), (char*)self->ident->buffer, INET_ADDRSTRLEN);
  
  ecbuf_format(self->ident, self->ident->size, "%s:%i", self->ident->buffer, self->senderAddr.sin_port);
  
  //eclogger_fmt (LL_TRACE, "ENTC", "datagram", "message from '%s'", self->ident->buffer);
   
  return count;
}

//-----------------------------------------------------------------------------------

size_t ecdatagram_write (EcDatagram self, size_t len)
{
  size_t count = sendto (self->socket->socket, self->buffer->buffer, len, 0,
                 (struct sockaddr*)&(self->senderAddr), self->senderAddrSize);
  
  return count;
}

//-----------------------------------------------------------------------------------

size_t ecdatagram_writeBuf (EcDatagram self, EcBuffer buf, size_t len)
{
  size_t count = sendto (self->socket->socket, buf->buffer, len, 0,
                          (struct sockaddr*)&(self->senderAddr), self->senderAddrSize);
  
  return count;  
}

//-----------------------------------------------------------------------------------

EcBuffer ecdatagram_buffer (EcDatagram self)
{
  return self->buffer;
}

//-----------------------------------------------------------------------------------

const EcString ecdatagram_ident (EcDatagram self)
{
  return ecbuf_const_str(self->ident);
}

//-----------------------------------------------------------------------------------

#endif
