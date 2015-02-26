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

#include "ecmail.h"

#include "../system/ecsocket.h"
#include "../utils/ecstreambuffer.h"

struct EcMail_s
{

  EcLogger mLogger;
  
  EcString mMailhost;
  
  uint_t mPort;
  
};

/*------------------------------------------------------------------------*/

EcMail ecmail_new(EcLogger logger, const EcString mailhost, uint_t port)
{
  EcMail self = ENTC_NEW(struct EcMail_s);
  
  self->mLogger = logger;
  
  self->mMailhost = ecstr_copy(mailhost);
  self->mPort = port;
  
  return self;
}

/*------------------------------------------------------------------------*/

void ecmail_delete(EcMail* ptr)
{
  ENTC_DEL(ptr, struct EcMail_s);
}

/*------------------------------------------------------------------------*/

int ecmail_next(EcMail self, const EcString txtto, const EcString txtre, const EcString subject, const EcString text, EcSocket socket, EcStreamBuffer buffer, EcStream stream)
{
  const EcString response;
  int error;
  
  char b1 = 0;
  char b2 = 0;
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "can't get initial handshake" );
    return FALSE;
  }
    
  response = ecstream_buffer(stream);
  // TODO: remove \n at the end of the message
  eclogger_logformat(self->mLogger, LL_TRACE, "MAIL", "handshake from '%s'", response );
  
  /* check if we have code 220 */
  if(!((response[0] == '2')&&(response[1] == '2')&&(response[2] == '0')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #1 '%s'", response );
    
    return FALSE;
  }
  /* now send our host */
  ecsocket_write( socket, "HELO me\n", 8 );
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "timeout receiving handshake" );
    return FALSE;
  }
  
  response = ecstream_buffer(stream);  
  /* check if we have code 250 */
  if(!((response[0] == '2')&&(response[1] == '5')&&(response[2] == '0')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #2 '%s'", response );    
    return FALSE;
  }
  
  /* send sender */
  ecsocket_write( socket, "MAIL FROM: ", 11 );
  ecsocket_write( socket, txtre, ecstr_len (txtre));
  ecsocket_write( socket, "\n", 1 );
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "timeout in accepting 'mail from'" );
    return FALSE;
  }
  
  response = ecstream_buffer(stream);
  /* check if we have code 250 */
  if(!((response[0] == '2')&&(response[1] == '5')&&(response[2] == '0')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #3 '%s'", response );
    return FALSE;
  }
  ecsocket_write( socket, "RCPT TO: ", 9 );
  ecsocket_write( socket, txtto, ecstr_len (txtto) );
  ecsocket_write( socket, "\n", 1 );
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "timeout in accepting 'rcpt from'" );
    return FALSE;
  }
  
  response = ecstream_buffer(stream);
  /* check if we have code 250 */
  if(!((response[0] == '2')&&(response[1] == '5')&&(response[2] == '0')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #4 '%s'", response );
    return FALSE;
  }
  
  ecsocket_write( socket, "DATA\n", 5 );
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "timeout in accepting 'data'" );
    return FALSE;
  }
  
  response = ecstream_buffer(stream);
  /* check if we have code 354 */
  if(!((response[0] == '3')&&(response[1] == '5')&&(response[2] == '4')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #5 '%s'", response );
    return FALSE;
  }
  
  /* content of the mail */
  ecsocket_write( socket, "From: ", 6 );
  ecsocket_write( socket, txtre, ecstr_len (txtre) );
  ecsocket_write( socket, "\nTo: ", 5 );
  ecsocket_write( socket, txtto, ecstr_len (txtto) );
  ecsocket_write( socket, "\nSubject: ", 10 );
  ecsocket_write( socket, subject, ecstr_len (subject) );
  ecsocket_write( socket, "\n", 1 );
  ecsocket_write( socket, text, ecstr_len (text) );
  ecsocket_write( socket, "\n.\n", 3 );
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "timeout in accepting 'mail'" );
    return FALSE;
  }
  
  response = ecstream_buffer(stream);
  /* check if we have code 250 */
  if(!((response[0] == '2')&&(response[1] == '5')&&(response[2] == '0')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #6 '%s'", response );
    return FALSE;
  }
  
  ecsocket_write( socket, "QUIT\n", 5 );
  
  if ( !ecstreambuffer_readln (buffer, stream, &error, &b1, &b2) )
  {
    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "timeout in accepting 'quit'" );
    return FALSE;
  }
  
  response = ecstream_buffer(stream);
  /* check if we have code 221 */
  if(!((response[0] == '2')&&(response[1] == '2')&&(response[2] == '1')))
  {
    eclogger_logformat(self->mLogger, LL_ERROR, "MAIL", "Protocol error #7 '%s'", response );
    return FALSE;
  }
  
  eclogger_log(self->mLogger, LL_INFO, "MAIL", "mail sent" );
  /* message was succesfully sent */
  return TRUE;  
}

/*------------------------------------------------------------------------*/

int ecmail_proceed(EcMail self, const EcString txtto, const EcString txtre, const EcString subject, const EcString text, EcSocket socket)
{
  // initialize a smart streambuffer to detect lines
  EcStreamBuffer buffer = ecstreambuffer_create (socket);
  // initialize stream to avoid buffer overflows
  EcStream stream = ecstream_new();

  int res = ecmail_next(self, txtto, txtre, subject, text, socket, buffer, stream);
  
  // clean up
  ecstream_delete(&stream);
  ecstreambuffer_destroy (&buffer);
  
  return res;
}

/*------------------------------------------------------------------------*/

int ecmail_send(EcMail self, const EcString txtto, const EcString txtre, const EcString subject, const EcString text, EcEventContext ec)
{
  EcSocket socket;
  int res = TRUE;
  
  socket = ecsocket_new(ec);
  
  eclogger_logformat(self->mLogger, LL_DEBUG, "MAIL", "try to connect to smtp server '%s:%u'", self->mMailhost, self->mPort );
  
  if( !ecsocket_connect(socket, self->mMailhost, self->mPort) )
  {
    // clean up
    ecsocket_delete(&socket);

    eclogger_log(self->mLogger, LL_ERROR, "MAIL", "can't connect to mailserver" );
    return FALSE;
  }
  
  res = ecmail_proceed(self, txtto, txtre, subject, text, socket);
  
  ecsocket_delete(&socket);
  
  return res;
}

/*------------------------------------------------------------------------*/
