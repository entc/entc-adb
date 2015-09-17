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

  int socket;
    
  EcString host;
  
};

//-----------------------------------------------------------------------------------

EcSocket ecsocket_new (EcEventContext ec)
{
  EcSocket self = ENTC_NEW(struct EcSocket_s);
  
  self->ec = ec;
  self->socket = 0;
  self->host = ecstr_init();
  
  return self;
}

//-----------------------------------------------------------------------------------

void ecsocket_delete (EcSocket* pself)
{
  EcSocket self = *pself;
    
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

int ecsocket_create (EcSocket self, const EcString host, uint_t port, int role)
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
    eclogger_errno (LL_ERROR, "ENTC", "socket", "Can't create socket");
    return -1;
  }
  
  if (role) 
  {
    if (connect(sock, (const struct sockaddr*)&(addr), sizeof(addr)) < 0) 
    {
      close(sock);      
      
      eclogger_errno (LL_ERROR, "ENTC", "socket", "Can't connect to '%s:%u'", host, port);
      return -1;
    }
  }
  else
  {
    int opt = 1; 
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
      eclogger_errno (LL_ERROR, "ENTC", "socket", "Can't set socket option");

      close(sock);
      return -1;
    }    
    
    if (bind(sock, (const struct sockaddr*)&(addr), sizeof(addr)) < 0) 
    {
      close(sock);
      
      eclogger_errno (LL_ERROR, "ENTC", "socket", "Can't bind to '%s:%u'", host, port);
      return -1;
    }
  }
  
  return sock;
}

//-----------------------------------------------------------------------------------

int ecsocket_connect (EcSocket self, const EcString host, uint_t port)
{
  self->socket = ecsocket_create(self, host, port, TRUE); 
  
  if (self->socket >= 0)
  {
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

int ecsocket_listen (EcSocket self, const EcString host, uint_t port)
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
  ecstr_replace(&(self->host), host);
  
  return TRUE;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_createReadSocket (EcEventContext ec, int sock, socklen_t addrlen, struct sockaddr* addr)
{
  EcSocket self = ENTC_NEW(struct EcSocket_s);
  // references
  self->ec = ec;
  // socket
  self->socket = sock;
  {
    EcBuffer ipbuffer = ecbuf_create (INET6_ADDRSTRLEN);    
    // convert address information into string    
    const char* address = inet_ntop (AF_INET, (const struct sockaddr_in *)addr, (char*)ipbuffer->buffer, INET_ADDRSTRLEN);
    if (isAssigned (address))
    {
      self->host = ecbuf_str (&ipbuffer);
      eclogger_fmt (LL_TRACE, "ENTC", "read socket", "connection accepted from '%s'", self->host);
    }
    else
    {
      address = inet_ntop (AF_INET6, addr, (char*)ipbuffer->buffer, INET6_ADDRSTRLEN);
      if (isAssigned (address))
      {
        self->host = ecbuf_str (&ipbuffer);
        eclogger_fmt (LL_TRACE, "ENTC", "read socket", "connection accepted from '%s'", self->host);
      }
      else
      {        
        ecbuf_destroy(&ipbuffer);
        self->host = ecstr_init ();
        
        eclogger_errno(LL_ERROR, "ENTC", "read socket", "can't get address string");
        eclogger_fmt (LL_TRACE, "ENTC", "read socket", "connection accepted");
      }
    }
    
  }
  return self;
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_accept (EcSocket self)
{
  while (TRUE)
  {
    struct sockaddr addr;
    socklen_t addrlen = 0;
    
    memset (&addr, 0x00, sizeof(addr));
    
    int sock = accept (self->socket, &addr, &addrlen);
    if (sock < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_errno (LL_ERROR, "ENTC", "socket", "Error on accept client connection");
        break;
      }
      else
      {
        eclogger_errno (LL_TRACE, "ENTC", "socket", "Minor rrror on accept client connection");
        continue;
      }
    }
    return ecsocket_createReadSocket (self->ec, sock, addrlen, &addr);
  }
  return NULL;
}

//-----------------------------------------------------------------------------------

int ecsocket_readBunch (EcSocket self, void* buffer, int nbyte)
{
  while (TRUE) 
  {
    int res = read (self->socket, buffer, nbyte);  
    if (res < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_errno (LL_ERROR, "ENTC", "socket read", "Error on recv");
        return -1;
      }
      else
      {
        continue;
      }
    }
    else if (res == 0)
    {
      eclogger_msg (LL_WARN, "ENTC", "socket read", "connection reset by peer");
      return 0;
    }    
    // end
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
    if (res < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_errno (LL_ERROR, "ENTC", "socket", "Error on send");
        return -1;
      }
      else
      {
        continue;
      }
    }
    else if (res == 0)
    {
      eclogger_msg (LL_ERROR, "ENTC", "socket write", "connection reset by peer");
      return 0;
    }    
    else
    {
      del += res;
    }    
  }
  return nbyte;  
}

//-----------------------------------------------------------------------------------

int ecsocket_writeStream (EcSocket self, EcStream stream)
{
  uint_t size = ecstream_size( stream );
  
  return ecsocket_write(self, ecstream_buffer(stream), size);    
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
  
  ecbuf_destroy (&buffer);
  
  return TRUE; 
}

//-----------------------------------------------------------------------------------

EcSocket ecsocket_acceptIntr (EcSocket self)
{
  while (TRUE) 
  {
    int sock;

    struct sockaddr addr;
    socklen_t addrlen = 0;
    
    memset (&addr, 0x00, sizeof(addr));
    // wait for either data on the handle or terminate signal
    int res = ece_context_wait (self->ec, self->socket, ENTC_INFINITE, ENTC_EVENTTYPE_READ);
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
    if (sock < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_errno (LL_ERROR, "ENTC", "socket", "Error on accept client connection");
        return NULL;
      }
      else
      {
        eclogger_errno (LL_TRACE, "ENTC", "socket", "Minor rrror on accept client connection");
        continue;
      }
    }
    return ecsocket_createReadSocket (self->ec, sock, addrlen, &addr);
  }  
}

//-----------------------------------------------------------------------------------

int ecsocket_readTimeout (EcSocket self, void* buffer, int nbyte, int timeout)
{
  while (TRUE) 
  {
    int res = ece_context_wait (self->ec, self->socket, timeout, ENTC_EVENTTYPE_READ);
    if (res == ENTC_EVENT_ABORT || res == ENTC_EVENT_TIMEOUT)
    {
      return res;
    }
    res = read (self->socket, buffer, nbyte); 
    if (res < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        eclogger_errno (LL_ERROR, "ENTC", "socket", "Error on recv");
        return -1;
      }
      else
      {
        continue;
      }
    }
    else if (res == 0)
    {
      eclogger_errno (LL_WARN, "ENTC", "socket", "connection reset by peer");
      return 0;
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
  return ecsocket_readTimeout (self, buffer, nbyte, timeout);  
}

//-----------------------------------------------------------------------------------

EcHandle ecsocket_getAcceptHandle (EcSocket self)
{
  return self->socket;
}

//-----------------------------------------------------------------------------------

EcHandle ecsocket_getReadHandle (EcSocket self)
{
  return self->socket;  
}

//-----------------------------------------------------------------------------------

void ecsocket_resetHandle (EcHandle handle)
{
  // not needed
}

//-----------------------------------------------------------------------------------

const EcString ecsocket_address (EcSocket self)
{
  return self->host;  
}

//-----------------------------------------------------------------------------------

#endif
