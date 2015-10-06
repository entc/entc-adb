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

#include "echtml.h"

#include "../system/ecsocket.h"
#include "../utils/ecstreambuffer.h"

/*------------------------------------------------------------------------*/

struct EcHtmlRequest_s
{

  EcStream stream;
  
};

/*------------------------------------------------------------------------*/

EcHtmlRequest echtmlreq_new (void)
{
  EcHtmlRequest self = ENTC_NEW(struct EcHtmlRequest_s);
  
  self->stream = ecstream_new();
  
  return self;
}

/*------------------------------------------------------------------------*/

void echtmlreq_delete(EcHtmlRequest* ptr)
{
  EcHtmlRequest self = *ptr;
  
  ecstream_delete( &(self->stream) );
  
  ENTC_DEL(ptr, struct EcHtmlRequest_s);
}

/*------------------------------------------------------------------------*/

void echtmlreq_escapeUrl(EcStream stream, const EcString source)
{
  /* variables */
  const char* pos01 = source;

  while(*pos01)
  {
    if( *pos01 == ' ' )
    {
      ecstream_append(stream, "%20");
    }
    else if( *pos01 == '#' )
    {
      ecstream_append(stream, "%23");
    }    
    else
    {
      ecstream_appendc(stream, *pos01);
    }
    pos01++;
  }
}

/*------------------------------------------------------------------------*/

int echtmlreq_process_post (EcHtmlRequest self, EcSocket socket, const EcString url, const EcString message, EcStream stream, EcStreamBuffer sbuffer)
{
  /* variables */
  const char* line;
  int error;
  
  char b1 = 0;
  char b2 = 0;

  ecstream_append(stream, "POST ");
  
  echtmlreq_escapeUrl(stream, url);

  ecstream_append(stream, " HTTP/1.0\r\n");
  ecstream_append(stream, "User-Agent: entc\r\n");
  ecstream_append(stream, "Content-Length: ");
  ecstream_appendu(stream, ecstr_len(message));
  ecstream_append(stream, "\r\n\r\n");
  
  ecstream_append(stream, message);

  ecstream_append(stream, "\r\n\r\n");

  ecsocket_writeStream(socket, stream);
  
  
  
  if( !ecstreambuffer_readln(sbuffer, stream, &error, &b1, &b2) )
  {
    eclogger_msg (LL_TRACE, "ENTC", "http", "can't get new line");    
    return FALSE;
  }  
  /* get the first line */
  line = ecstream_buffer(stream);  
  /* is the first line 'HTTP/1.0 200 OK' ?? */
  if( !(line[0] == 'H' && line[9] == '2' && line[10] == '0' && line[11] == '0') )
  {
    eclogger_fmt (LL_TRACE, "ENTC", "http", "line : '%s'", line);    
    return FALSE;
  }
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

int echtmlreq_process(EcHtmlRequest self, EcSocket socket, const EcString host, const EcString url, EcStream stream, EcStreamBuffer sbuffer)
{
  /* variables */
  const char* line;
  int error;
  
  char b1 = 0;
  char b2 = 0;
  
  /* set to none blocking socket */
//  ecsocket_setNoneBlocking( socket );
  
  ecstream_append(stream, "GET ");

  echtmlreq_escapeUrl(stream, url);

  ecstream_append(stream, " HTTP/1.0\r\nUser-Agent: entc\r\nAccept: */*\r\nHost: ");
  ecstream_append(stream, host);
  ecstream_append(stream, "\r\nConnection: Keep-Alive\r\n\r\n");
  
  /* send the request */
  ecsocket_writeStream(socket, stream);
  
  if( !ecstreambuffer_readln(sbuffer, stream, &error, &b1, &b2) )
  {
    eclogger_msg (LL_TRACE, "ENTC", "http", "can't get new line");    
    return FALSE;
  }
  /* get the first line */
  line = ecstream_buffer(stream);  
  /* is the first line 'HTTP/1.0 200 OK' ?? */
  if( !(line[0] == 'H' && line[9] == '2' && line[10] == '0' && line[11] == '0') )
  {
    eclogger_fmt (LL_TRACE, "ENTC", "http", "line : '%s'", line);    
    return FALSE;
  }
  /* read all header lines */
  while( ecstreambuffer_readln(sbuffer, stream, &error, &b1, &b2) )
  {
    line = ecstream_buffer(stream);
    if( !*line )
    {
      break;
    }      
  }
  /* read all data lines */
  ecstreambuffer_read(sbuffer, self->stream, &error);
  
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

int echtmlreq_get(EcHtmlRequest self, const EcString host, uint_t port, const EcString url, EcEventContext ec)
{
  int res = FALSE;
  
  EcSocket socket;
  
  socket = ecsocket_new (ec, ENTC_SOCKET_PROTOCOL_TCP);
  
  if( ecsocket_connect( socket, host, port ) )
  {
    EcString cport = ecstr_long(port);
    EcString chost = ecstr_cat3(host, ":", cport);
    
    EcStream stream = ecstream_new();    
    EcStreamBuffer sbuffer = ecstreambuffer_create (socket);
    
    if (echtmlreq_process (self, socket, chost, url, stream, sbuffer))
    {
      res = TRUE;
    }
    
    ecstreambuffer_destroy (&sbuffer);
    ecstream_delete(&stream);
    
    ecstr_delete(&chost);
    ecstr_delete(&cport);
  }
  else
  {
    eclogger_fmt (LL_DEBUG, "ENTC", "http", "can't connect '%s'", host );    
  }
  
  ecsocket_delete(&socket);
  
  return res;
}

/*------------------------------------------------------------------------*/

int echtmlreq_post (EcHtmlRequest self, const EcString host, uint_t port, const EcString url, const EcString message, EcEventContext ec)
{
  int res = FALSE;
  
  EcSocket socket;
  
  socket = ecsocket_new (ec, ENTC_SOCKET_PROTOCOL_TCP);
  
  if( ecsocket_connect( socket, host, port ) )
  {
    EcString cport = ecstr_long(port);
    EcString chost = ecstr_cat4("http://", host, ":", cport);
    EcString curl = ecstr_cat2(chost, url);

    EcStream stream = ecstream_new();    
    EcStreamBuffer sbuffer = ecstreambuffer_create (socket);

    echtmlreq_process_post (self, socket, curl, message, stream, sbuffer);

    ecstreambuffer_destroy (&sbuffer);
    ecstream_delete(&stream);

    ecstr_delete(&chost);
    ecstr_delete(&cport);
    ecstr_delete(&curl);
  }
  else
  {
    eclogger_fmt (LL_DEBUG, "ENTC", "http", "can't connect '%s'", host );    
  }
  
  ecsocket_delete(&socket);
  
  return res;  
}

/*------------------------------------------------------------------------*/

const EcString echtmlreq_data(EcHtmlRequest self)
{
  return ecstream_buffer( self->stream );
}

/*------------------------------------------------------------------------*/
