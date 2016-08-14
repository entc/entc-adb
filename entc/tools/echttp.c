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

#include "echttp.h"

#include <utils/ecreadbuffer.h>
#include <utils/ecsecfile.h>
#include <utils/ecmessages.h>
#include <types/ecmapchar.h>
#include <system/ectime.h>

#include <string.h>
#include <fcntl.h>

static EcMapChar mime_types = NULL;
EcBuffer session_name;

#define _ENTC_SESSION_NAMELENGTH 23

static int echhtp_counter = 0;


#define _ENTC_MAX_BUFFERSIZE 32000 //  32kb

struct EcHttpContent_s
{
  // owned
  EcBuffer buffer;
  
  EcString filename;
  
  EcString path;
  
};

//---------------------------------------------------------------------------------------

void echttp_content_newRandomFile (EcHttpContent self)
{
  EcBuffer h = ecbuf_create (32);
  
  ecbuf_random(h, 32);
  
  self->filename = ecfs_mergeToPath (self->path, (char*)h->buffer);
  
  ecbuf_destroy (&h);        
}

//---------------------------------------------------------------------------------------

char* _STDCALL echttp_content_callback_mm (void* ptr, char* buffer, ulong_t inSize, int* outRes)
{
  EcStreamBuffer bstream = ptr;
  
  void* data = ecstreambuffer_getBunch (bstream, inSize, outRes);
  
  if (*outRes > 0)
  {
    memcpy (buffer, data, *outRes);
  }
  
  return data;
}

//---------------------------------------------------------------------------------------

char* _STDCALL echttp_content_callback_bf (void* ptr, char* buffer, ulong_t inSize, int* outRes)
{
  EcStreamBuffer bstream = ptr;
  
  return ecstreambuffer_getBunch (bstream, inSize, outRes);
}

//---------------------------------------------------------------------------------------

int echttp_content_fillFile (EcHttpContent self, ulong_t size, http_content_callback bf, void* ptr)
{
  EcFileHandle fh;
  ulong_t read = 0;

  self->buffer = ecbuf_create (_ENTC_MAX_BUFFERSIZE);
  
  echttp_content_newRandomFile (self);

  // open a new file
  fh = ecfh_open (self->filename, O_WRONLY | O_CREAT); 
  if (isNotAssigned (fh))
  {
    eclogger_errno (LL_ERROR, "ENTC", "http", "can't open file '%s'", self->filename);
    return FALSE;
  }
  
  while (read < size)
  {
    int res = 0;
    
    ulong_t diff = size - read;
    
    void* data = bf (ptr, (char*)self->buffer->buffer, diff < _ENTC_MAX_BUFFERSIZE ? diff : _ENTC_MAX_BUFFERSIZE, &res);    
    if (res > 0)
    {
      if (ecfh_writeConst (fh, data, res) != res)
      {
        eclogger_errno (LL_ERROR, "ENTC", "http", "can't write file '%s'", self->filename);
        
        ecfh_close (&fh);  
        return FALSE;            
      }
      
      read += res;          
    }
    else
    {
      break;
    }
  }
  
  ecfh_close (&fh);  
  
  ecbuf_destroy (&(self->buffer));
  
  eclogger_fmt(LL_TRACE, "ENTC", "content", "created new temporary file '%s' with size %i", self->filename, size);
  
  return TRUE;
}

//---------------------------------------------------------------------------------------

int echttp_content_fillBuffer (EcHttpContent self, ulong_t size, http_content_callback mm, void* ptr)
{
  // setup
  self->filename = NULL;
  self->buffer = ecbuf_create (size);
  {
    ulong_t read = 0;
    
    while (read < size)
    {
      int res = 0;
      
      mm (ptr, (char*)self->buffer->buffer + read, size - read, &res);      
      if (res > 0)
      {          
        read += res;          
      }
      else
      {
        break;
      }
    }
  }
  return TRUE;
}

//---------------------------------------------------------------------------------------

void echttp_setPath (EcHttpContent self, const EcString path)
{
  self->path = ecstr_copy (path);
  
  ecfs_createDirIfNotExists (self->path);
}

//---------------------------------------------------------------------------------------

EcHttpContent echttp_content_create (ulong_t size, http_content_callback bf, http_content_callback mm, void* ptr, const EcString path)
{
  EcHttpContent self = ENTC_NEW (struct EcHttpContent_s);
  
  self->path = ecstr_init ();
  
  if (size > _ENTC_MAX_BUFFERSIZE)
  {
    //eclogger_fmt(LL_TRACE, "ENTC", "content", "read using temporary file");

    echttp_setPath (self, path);

    if (!echttp_content_fillFile (self, size, bf, ptr))
    {
      echttp_content_destroy (&self);
      return NULL;      
    }
  }
  else
  {
    //eclogger_fmt(LL_TRACE, "ENTC", "content", "read using buffer");

    if (!echttp_content_fillBuffer (self, size, mm, ptr))
    {
      echttp_content_destroy (&self);
      return NULL;      
    }
  }
  
  return self;
}

//---------------------------------------------------------------------------------------

void echttp_content_destroy (EcHttpContent* pself)
{
  EcHttpContent self = *pself;
  
  if (isAssigned (self->buffer))
  {
    ecbuf_destroy(&(self->buffer));
  }

  if (isAssigned (self->filename))
  {
    ecfs_rmfile (self->filename);
    
    ecstr_delete(&(self->filename));
  }

  ecstr_delete (&(self->path));
  
  ENTC_DEL (pself, struct EcHttpContent_s);
}

//---------------------------------------------------------------------------------------

int echttp_content_hasBuffer (EcHttpContent self)
{
  if (isNotAssigned (self))
  {
    return FALSE;
  }
  
  return isAssigned (self->buffer);
}

//---------------------------------------------------------------------------------------

int echttp_content_hasFile (EcHttpContent self)
{
  if (isNotAssigned (self))
  {
    return FALSE;
  }
    
  return isAssigned (self->filename);
}

//---------------------------------------------------------------------------------------

EcString echttp_content_getFile (EcHttpContent self)
{
  if (isAssigned (self->filename))
  {
    return self->filename;
  }
  else if (isAssigned (self->buffer))
  {
    // variables
    EcFileHandle fh;

    echttp_content_newRandomFile (self);
    
    // open a new file
    fh = ecfh_open (self->filename, O_WRONLY | O_CREAT); 
    if (isAssigned (fh))
    {
      if (ecfh_writeBuffer (fh, self->buffer, self->buffer->size) != self->buffer->size)
      {
        eclogger_errno (LL_ERROR, "ENTC", "http", "can't write file '%s'", self->filename);
        
        ecfh_close (&fh);
        return NULL;        
      }

      ecfh_close (&fh);
      
      return self->filename;
    }
  }
  return NULL;
}

//---------------------------------------------------------------------------------------

EcBuffer echttp_content_getBuffer (EcHttpContent self)
{
  return self->buffer;
}

//---------------------------------------------------------------------------------------

void echttp_init (void)
{
  if (echhtp_counter == 0)
  {
    session_name = ecbuf_create (_ENTC_SESSION_NAMELENGTH);  
    ecbuf_random (session_name, _ENTC_SESSION_NAMELENGTH);
    
    mime_types = ecmapchar_create (EC_ALLOC); 
    
    ecmapchar_append( mime_types, "pdf",      "application/pdf" );
    ecmapchar_append( mime_types, "sig",      "application/pgp-signature" );
    ecmapchar_append( mime_types, "class",    "application/octet-stream" );
    ecmapchar_append( mime_types, "ps",       "application/postscript" );
    ecmapchar_append( mime_types, "torrent",  "application/x-bittorrent" );
    ecmapchar_append( mime_types, "dvi",      "application/x-dvi" );
    ecmapchar_append( mime_types, "gz",       "application/x-gzip" );
    ecmapchar_append( mime_types, "pac",      "application/x-ns-proxy-autoconfig" );
    ecmapchar_append( mime_types, "swf",      "application/x-shockwave-flash" );
    
    ecmapchar_append( mime_types, "tgz",      "application/x-tgz" );
    ecmapchar_append( mime_types, "tar",      "application/x-tar" );
    ecmapchar_append( mime_types, "zip",      "application/zip" );
    ecmapchar_append( mime_types, "mp3",      "audio/mpeg" );
    ecmapchar_append( mime_types, "m3u",      "audio/x-mpegurl" );
    ecmapchar_append( mime_types, "wma",      "audio/x-ms-wma" );
    ecmapchar_append( mime_types, "wax",      "audio/x-ms-wax" );
    ecmapchar_append( mime_types, "ogg",      "application/ogg" );
    ecmapchar_append( mime_types, "wav",      "audio/x-wav" );
    
    ecmapchar_append( mime_types, "gif",      "image/gif" );
    ecmapchar_append( mime_types, "jpg",      "image/jpeg" );
    ecmapchar_append( mime_types, "jpeg",     "image/jpeg" );
    ecmapchar_append( mime_types, "png",      "image/png" );
    ecmapchar_append( mime_types, "xbm",      "image/x-xbitmap" );
    ecmapchar_append( mime_types, "xpm",      "image/x-xpixmap" );
    ecmapchar_append( mime_types, "xwd",      "image/x-xwindowdump" );
    ecmapchar_append( mime_types, "svg",      "image/svg+xml");

    ecmapchar_append( mime_types, "css",      "text/css" );
    ecmapchar_append( mime_types, "html",     "text/html" );
    ecmapchar_append( mime_types, "htm",      "text/html" );
    ecmapchar_append( mime_types, "js",       "text/javascript" );
    ecmapchar_append( mime_types, "asc",      "text/plain" );
    ecmapchar_append( mime_types, "c",        "text/plain" ); 
    ecmapchar_append( mime_types, "ico",      "image/x-ico; charset=binary" );
    ecmapchar_append( mime_types, "map",      "application/json");

    ecmapchar_append( mime_types, "woff",     "application/font-woff");
    ecmapchar_append( mime_types, "woff2",    "application/font-woff2");
    ecmapchar_append( mime_types, "ttf",      "application/font-ttf");
    ecmapchar_append( mime_types, "eot",      "application/vnd.ms-fontobject");
    ecmapchar_append( mime_types, "otf",      "application/font-otf");

    ecmapchar_append( mime_types, "csv",      "text/csv");
    ecmapchar_append( mime_types, "xls",      "application/vnd.ms-excel");

    ecmapchar_append( mime_types, "exe",      "application/octet-stream" );
    ecmapchar_append( mime_types, "dll",      "application/x-msdownload" );
  }
  echhtp_counter++;
}

//---------------------------------------------------------------------------------------

void echttp_done (void)
{
  echhtp_counter--;
  if (echhtp_counter == 0) 
  {
    ecmapchar_destroy (EC_ALLOC, &mime_types);
    ecbuf_destroy (&session_name);
  }
}

//---------------------------------------------------------------------------------------

const EcString echttp_url_lastPart (const EcString url)
{
  const char* pos = strrchr (url, '/');
  if (pos)
  {
    return pos + 1;
  }
  else
  {
    return url;
  }
}

//---------------------------------------------------------------------------------------

const EcString echttp_getMimeType (const EcString filename)
{
  /* variables */
  const EcString pos01;
  const EcString pos02;
  EcMapCharNode node;
  
  pos01 = ecfs_extractFile(filename);
  
  if( !pos01 )
  {
    eclogger_fmt (LL_WARN, "ENTC", "mime", "can't extract filename from '%s'", filename);
    
    return "application/octet-stream";
  }
  
  /* extract the extension from the file */
  pos02 = strrchr(pos01, '.');
  
  if( !pos02 )
  {
    eclogger_fmt (LL_WARN, "ENTC", "mime", "can't find file extension from '%s'", filename);
    
    return "application/octet-stream";
  }
  
  node = ecmapchar_find( mime_types, pos02 + 1 );
  if( node != ecmapchar_end( mime_types ))
  {
    return ecmapchar_data( node );  
  }
  else
  {
    eclogger_fmt (LL_WARN, "ENTC", "mime", "can't find mime type '%s' from '%s'", pos02 + 1, filename);
    return "application/octet-stream";
  }   
}

//---------------------------------------------------------------------------------------

void echttp_send_status (EcHttpHeader* header, EcDevStream stream, const EcString code)
{
  if (header->header_on)
  {
    ecdevstream_appends( stream, "HTTP/1.1 ");
    ecdevstream_appends( stream, code );
    ecdevstream_appends( stream, "\r\n" );
    ecdevstream_appends( stream, "Server: " );
    ecdevstream_appends( stream, "entc" );
    ecdevstream_appends( stream, "\r\n" );
  }
  else
  {
    // for cgi or fcgi header result
    ecdevstream_appends( stream, "Status: ");
    ecdevstream_appends( stream, code );
    ecdevstream_appends( stream, "\r\n" );      
  }  
}

//---------------------------------------------------------------------------------------

void echttp_send_params (EcDevStream stream, EcUdc extra_params)
{
  // add extra stuff
  if (isAssigned (extra_params))
  {
    void* cursor = NULL;
    EcUdc item;
    
    for (item  = ecudc_next (extra_params, &cursor); isAssigned (item); item = ecudc_next (extra_params, &cursor))
    {
      ecdevstream_appends( stream, ecudc_name (item));
      ecdevstream_appends( stream, ": ");
      ecdevstream_appends( stream, ecudc_asString(item));
      ecdevstream_appends( stream, "\r\n");
    }
  }  
}

//---------------------------------------------------------------------------------------

void echttp_send_header (EcHttpHeader* header, EcDevStream stream, const EcString code, EcUdc extra_params)
{
  echttp_send_status (header, stream, code);
  
  if (!ecstr_empty(header->mime))
  {
    ecdevstream_appends( stream, "Content-type: ");
    ecdevstream_appends( stream, header->mime);
    ecdevstream_appends( stream, "\r\n" );    
  }
  
  if (!ecstr_empty(header->session_lang))
  {
    ecdevstream_appends( stream, "Content-Language: ");
    ecdevstream_appends( stream, header->session_lang );
    ecdevstream_appends( stream, "\r\n" );  
  }
  
  /*
  if (ecstr_valid(header->sessionid))
  {
    EcString strtime = ecstr_localtime(time(0) + 86400);
    ecdevstream_appends( stream, "Set-cookie:" );
    ecdevstream_appends( stream, ecstr_get(session_name) );
    ecdevstream_appendc( stream, '=' );
    ecdevstream_appends( stream, header->sessionid );
    ecdevstream_appendc( stream, '-' );
    ecdevstream_appends( stream, header->session_lang );
    ecdevstream_appends( stream, "; expires=" );
    ecdevstream_appends( stream, strtime );
    ecdevstream_appends( stream, "; path=/\r\n" );
    ecstr_delete(&strtime); 
  }
   */
  
  /*
  if (isAssigned (header->auth))
  {
    void* cursor = NULL;
    EcUdc item;
    int counter = 0;
    
    ecdevstream_appends ( stream, "Authorization: ");
    ecdevstream_appends ( stream, ecudc_name(header->auth));
    ecdevstream_appendc (stream, ' ');
    
    for (item  = ecudc_next (header->auth, &cursor); isAssigned (item); item = ecudc_next (header->auth, &cursor), counter++)
    {
      if (counter > 0)
      {
        ecdevstream_appendc (stream, ',');
      }
      ecdevstream_appends (stream, ecudc_name (item));
      ecdevstream_appends (stream, "=\"");
      ecdevstream_appends (stream, ecudc_asString (item));
      ecdevstream_appendc (stream, '"');
    }
    
    ecdevstream_appends( stream, "\r\n" );
  }
   */
  
  echttp_send_params (stream, extra_params);
  
  // finish the header part
  ecdevstream_appends( stream, "\r\n" );
}

//---------------------------------------------------------------------------------------

void echttp_send_ErrHeader (EcHttpHeader* header, EcDevStream stream, ulong_t errcode, EcUdc extra_params)
{
  switch (errcode)
  {
    case ENTC_RESCODE_NEEDS_AUTH:
    case ENTC_RESCODE_CLEAR_AUTH:
    {
      //if (isAssigned (header->auth))
      //{
      //  echttp_send_status (header, stream, "403 Forbidden");      
      //}
      //else
      {
        echttp_send_status (header, stream, "401 Unauthorized");        
      }
    }
    break;
    case ENTC_RESCODE_NEEDS_PERMISSION:
    {
      echttp_send_status (header, stream, "403 Forbidden");      
    }
    break;
    case ENTC_RESCODE_ALREADY_EXISTS:
    {
      echttp_send_status (header, stream, "405 Method Not Allowed");
    }
    break;
    case ENTC_RESCODE_NEEDS_CRYPT:
    {
      echttp_send_status (header, stream, "418 I’m a teapot");      
    }
    break;
    case ENTC_RESCODE_REQUEST_TOOBIG:
    {
      echttp_send_status (header, stream, "413 Request Entity Too Large");      
    }
    break;
    default:
    {
      echttp_send_status (header, stream, "404 Not Found");            
    }
    break;
  }
  
  echttp_send_params (stream, extra_params);
  
  // finish
  ecdevstream_append( stream, "\r\n", 2 );
  
  eclogger_fmt (LL_TRACE, "ENTC", "send http", "send error http message [%i]", errcode);
}

//---------------------------------------------------------------------------------------

void echttp_send_DefaultHeader (EcHttpHeader* header, EcDevStream stream, EcUdc extra_params)
{
  echttp_send_header (header, stream, "200 OK", extra_params);
}

//---------------------------------------------------------------------------------------

void q4http_callback(void* ptr, const void* buffer, uint_t nbyte)
{
  ecsocket_write (ptr, buffer, nbyte);
}

//---------------------------------------------------------------------------------------

void echttp_send_408 (EcHttpHeader* header, EcSocket socket)
{  
  if (header->header_on)
  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket); // output stream
    // send back that the file doesn't exists
    ecdevstream_appends( stream, "HTTP/1.1 408 Request Timeout\r\n" );
    ecdevstream_appends( stream, "\r\n" );

    ecdevstream_delete( &stream );  
  }
}

//---------------------------------------------------------------------------------------

void echttp_send_417 (EcHttpHeader* header, EcSocket socket)
{  
  if (header->header_on)
  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket); // output stream
    // send back that the file doesn't exists
    ecdevstream_appends( stream, "HTTP/1.1 417 Expectation Failed\r\n" );
    ecdevstream_appends( stream, "\r\n" );
    
    ecdevstream_delete( &stream );  
  }
}

//---------------------------------------------------------------------------------------

void echttp_send_100 (EcHttpHeader* header, EcSocket socket)
{  
  if (header->header_on)
  {
    EcDevStream stream = ecdevstream_new (1024, q4http_callback, socket); // output stream
    // send back that the file doesn't exists
    ecdevstream_appends ( stream, "HTTP/1.1 100 Continue\r\n" );
    ecdevstream_appends ( stream, "\r\n" );
    
    ecdevstream_delete ( &stream );  
  }
}

//---------------------------------------------------------------------------------------

void echttp_send_500 (EcHttpHeader* header, EcSocket socket)
{  
  if (header->header_on)
  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket); // output stream
    // send back that the file doesn't exists
    ecdevstream_appends( stream, "HTTP/1.1 500 Internal Server Error\r\n" );
    ecdevstream_appends( stream, "\r\n" );
    
    ecdevstream_delete( &stream );  
  }
}

//---------------------------------------------------------------------------------------

void echttp_send_Invalid (EcHttpHeader* header, EcSocket socket)
{
  if (header->header_on)
  {

  }
}

//---------------------------------------------------------------------------------------

void echttp_send_404NotFound (EcHttpHeader* header, EcSocket socket, const EcString message)
{
  if (header->header_on)
  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket); // output stream
    // send back that the file doesn't exists
    ecdevstream_appends( stream, "HTTP/1.1 404 Not Found\r\n" );
    ecdevstream_appends( stream, "\r\n" );
    if (ecstr_valid(message))
    {
      ecdevstream_appends( stream, "<html><body>");
      ecdevstream_appends( stream,message);               
      ecdevstream_appends( stream, "</body></html>\r\n" );
    }
    ecdevstream_delete( &stream );
  }                     
}

//---------------------------------------------------------------------------------------

void echttp_send_SecureIncidentStream (EcHttpHeader* header, EcDevStream stream)
{
  echttp_send_DefaultHeader (header, stream, NULL);
  
  /* render html part */
  ecdevstream_appends( stream, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n\t\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n" );
  
  ecdevstream_appends( stream, "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n" );
  
  ecdevstream_appends( stream, "<body>\n" );
  ecdevstream_appends( stream, "<font color=\"#ff0000\" size=\"+4\">Security incident reported!</font><br><br>\n" );
  ecdevstream_appends( stream, "identified by " );
  ecdevstream_appends( stream, ecstr_cstring(header->remote_address) );
  ecdevstream_appends( stream, "</body>\n</html>" );  
}

//---------------------------------------------------------------------------------------

void echttp_send_SecureIncident (EcHttpHeader* header, EcSocket socket)
{
  EcDevStream devstream = ecdevstream_new (1024, q4http_callback, socket);

  echttp_send_SecureIncidentStream (header, devstream);
                   
  ecdevstream_delete (&devstream);
}

//---------------------------------------------------------------------------------------

void echttp_send_file (EcHttpHeader* header, EcSocket socket, const EcString docroot)
{
  // variables
  EcString filename = ecstr_copy (header->request_url);
  EcString path;
  struct EcSecFopen secopen;
  int res;
  
  if (ENTC_PATH_SEPARATOR != '/')
  {
    ecstr_replaceAllChars (filename, '/', ENTC_PATH_SEPARATOR);
  }
  
  eclogger_fmt (LL_TRACE, "ENTC", "http", "try to send file '%s'", filename);

  path = ecfs_mergeToPath(docroot, filename);
  
  //eclogger_logformat(logger, LOGMSG_INFO, "SERV", "try to access '%s'", path);

  // try to open filename
  res = ecsec_fopen (&secopen, path, O_RDONLY, docroot);
  // cleanup
  ecstr_delete(&path); 
  
  if (res == FALSE) 
  {
    ecstr_delete(&filename);
    ecstr_delete(&(secopen.filename));

    /*
    if (secopen.sec_error != 0) 
    {
      echttp_send_SecureIncident (header, socket);
      return;
    }
     */
    echttp_send_404NotFound (header, socket, ecstr_init());    
    return;
  }

  header->mime = echttp_getMimeType (filename);
  
  //eclogger_logformat(logger, LOGMSG_INFO, "SERV", "send file '%s' : '%s'", filename, hr->mime );
  
  ecstr_delete(&filename); 

  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket);  // output stream for header
    
    echttp_send_DefaultHeader (header, stream, NULL);

    ecdevstream_delete( &stream ); 
    
    ecsocket_writeFile(socket, secopen.fhandle);
    
    ecfh_close( &(secopen.fhandle) );  
  }
  
  ecstr_delete(&(secopen.filename));
}

//---------------------------------------------------------------------------------------

void echttp_header_init (EcHttpHeader* header, int header_on)
{
  header->method = ecstr_init();
  header->header_on = header_on;
  header->header_only = 0;
  header->host = ecstr_init();
  header->remote_address = ecstr_init();
  header->user_lang = ecstr_init();
  header->user_agent = ecstr_init();
  header->session_lang = ecstr_init();
  header->url = NULL;
  header->request_url = ecstr_init();
  header->request_params = ecstr_init(); 
  header->mime = ecstr_init();
  header->title = ecstr_init();
  header->tokens = NULL;
  header->token = NULL;
  header->urlpath = ecstr_init();
  header->content_length = 0;
  header->content = NULL;
  header->sessionid = ecstr_init();
  header->auth = NULL;
  header->values = ecmapchar_create (EC_ALLOC);
}

//---------------------------------------------------------------------------------------

void echttp_header_clear (EcHttpHeader* header)
{
  ecstr_delete(&(header->method));
  ecstr_delete(&(header->host));
  ecstr_delete(&(header->user_lang));
  ecstr_delete(&(header->user_agent));
  ecstr_delete(&(header->session_lang));
  ecstr_delete(&(header->request_params));
  ecstr_delete(&(header->request_url));
  ecstr_delete(&(header->title));
  ecstr_delete(&(header->url));
  
  if (isAssigned (header->tokens))
  {
    ecstr_tokenizer_clear(header->tokens);
    eclist_free_ex (EC_ALLOC, &(header->tokens));
  }  
  ecstr_delete(&(header->urlpath));
  
  if (isAssigned (header->content))
  {
    echttp_content_destroy (&(header->content));
  }
    
  ecstr_delete(&(header->sessionid));
  
  if (isAssigned (header->auth))
  {
    ecudc_destroy(EC_ALLOC, &(header->auth));
  }
  
  ecmapchar_destroy (EC_ALLOC, &(header->values));
}

//---------------------------------------------------------------------------------------

char x2c (char *what)
{
  register char digit;
  
  digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
  return(digit);
}

//---------------------------------------------------------------------------------------

void echttp_unescape (EcString url)
{
  register int x,y;
  
  for(x=0;url[x];x++)
    if(url[x] == '%')
      url[x+1] = x2c(&url[x+1]);
  
  for(x=0,y=0;url[y];++x,++y) {
    if((url[x] = url[y]) == '%') {
      url[x] = url[y+1];
      y+=2;
    }
  }
  url[x] = '\0';
}

//---------------------------------------------------------------------------------------

void echttp_escape (EcDevStream stream, const EcString url)
{
  /* variables */
  const char* pos01 = url;
  
  while(*pos01)
  {
    switch (*pos01)
    {
      case ' ' : ecdevstream_appends(stream, "%20"); break;
      case '!' : ecdevstream_appends(stream, "%21"); break;
      case '"' : ecdevstream_appends(stream, "%22"); break;
      case '#' : ecdevstream_appends(stream, "%23"); break;
      case '$' : ecdevstream_appends(stream, "%24"); break;
      case '%' : ecdevstream_appends(stream, "%25"); break;
      case '&' : ecdevstream_appends(stream, "%26"); break;
      case '\'': ecdevstream_appends(stream, "%27"); break;
      default: ecdevstream_appendc(stream, *pos01);
    }
    pos01++;
  }
}

//---------------------------------------------------------------------------------------

void echttp_escape_stream (EcStream stream, const EcString url)
{
  /* variables */
  const char* pos01 = url;
  
  while(*pos01)
  {
    switch (*pos01)
    {
      case ' ' : ecstream_append (stream, "%20"); break;
      case '!' : ecstream_append (stream, "%21"); break;
      case '"' : ecstream_append (stream, "%22"); break;
      case '#' : ecstream_append (stream, "%23"); break;
      case '$' : ecstream_append (stream, "%24"); break;
      case '%' : ecstream_append (stream, "%25"); break;
      case '&' : ecstream_append (stream, "%26"); break;
      case '\'': ecstream_append (stream, "%27"); break;
      default: ecstream_appendc (stream, *pos01);
    }
    pos01++;
  }
}

//-----------------------------------------------------------------------------------------------------------

void echttp_url (EcHttpHeader* header, EcDevStream stream, const EcString url)
{
  if (ecstr_valid (header->host))
  {
    ecdevstream_appends( stream, "http://" ); 
    ecdevstream_appends( stream, header->host); 
    ecdevstream_appends( stream, "/" ); 
    echttp_escape (stream, url); 
  }
}

//-----------------------------------------------------------------------------------------------------------

void echttp_realurl (EcHttpHeader* header, EcDevStream stream, const EcString url)
{
  if( strlen(url) > 7 )
  {
    if( (url[0] == 'h') && (url[1] == 't') && (url[2] == 't') && (url[3] == 'p') && (url[4] == ':') )
    {
      echttp_escape (stream, url);
      return;
    }
  }
  echttp_url (header, stream, url);
}

//---------------------------------------------------------------------------------------

void echttp_header_title (EcHttpHeader* header)
{
  // variables
  EcListNode node;
  EcStream stream = ecstream_new ();
  // cast
  EcString url_unescaped = ecstr_copy (header->request_url);
  // unescape html url
  echttp_unescape (url_unescaped);
  
  header->tokens = eclist_create_ex (EC_ALLOC);
  // split url into parts
  ecstr_tokenizer (header->tokens, url_unescaped, '/');
  // clean up
  ecstr_delete (&url_unescaped);  
  
  for (node = eclist_first(header->tokens); node != eclist_end(header->tokens); node = eclist_next(node))
  {
    EcString token = eclist_data (node);

    if (node != eclist_first(header->tokens))
    {
      ecstream_append(stream, " - ");
    }
    
    ecstream_append(stream, token);
  }
  
  {
    EcBuffer buffer = ecstream_trans (&stream);
    header->title = ecbuf_str (&buffer);
  }
}

//---------------------------------------------------------------------------------------

void echttp_header_trimurl (EcHttpHeader* header)
{
  // extract host part
  if ((header->request_url[0] == '/')) 
  {
    if ((header->request_url[1] == '/')) 
    {
      // we do have a complete url, remove the first part
      const EcString pos = strchr (header->request_url + 2, '/');  
      if( pos )
      {
        ecstr_replace (&(header->request_url), pos);        
      }
    }
  }
  
  echttp_unescape (header->request_url);
  
  ecstr_replaceTO(&(header->request_url), ecstr_trimc (header->request_url, '/'));  
}

//---------------------------------------------------------------------------------------

void echttp_header_validate (EcHttpHeader* header)
{
  if (ecstr_empty (header->url))
  {
    ecstr_replace(&(header->request_url), "index.html"); 
  }
  else
  {
    // search for '?' in url
    const EcString pos01 = strchr(header->url, '?');  
    if( pos01 )
    {
      ecstr_replaceTO(&(header->request_url), ecstr_part(header->url, pos01 - header->url));
      ecstr_replace(&(header->request_params), pos01 + 1);
    }
    else
    {
      ecstr_replace(&(header->request_url), header->url); 
    }    
    
    echttp_header_trimurl (header);
  }
  
  eclogger_fmt(LL_TRACE, "ENTC", "http header", "request host: '%s'", header->host);
  eclogger_fmt(LL_TRACE, "ENTC", "http header", "request url: '%s'", header->request_url);
  
  // set correct language
  if (ecstr_empty(header->session_lang)) 
  {
    if (ecstr_empty(header->user_lang)) 
    {
      ecstr_replace(&(header->session_lang), "en");
    }
    else
    {
      ecstr_replace(&(header->session_lang), header->user_lang);
    }
  }
  // automatic create a sessionid
  if (ecstr_empty (header->sessionid))
  {
    EcBuffer buffer = ecbuf_create (_ENTC_SESSION_NAMELENGTH);
    // create a new sessionid
    ecbuf_random (buffer, _ENTC_SESSION_NAMELENGTH);
    // transfer ownership to string
    header->sessionid = ecbuf_str (&buffer);
  }
}

//---------------------------------------------------------------------------------------

void echttp_header_lotToService (EcHttpHeader* header)
{
  EcMessageData data;
  
  data.type = Q5_MSGTYPE_HTTP_REQUEST_INFO; data.rev = 1;
  data.ref = 0;
  
  data.content = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, "AccessInfo");
    
  // generate the udc containers with all the infos we have :-)
  {
    EcUdc item = ecudc_create(EC_ALLOC, 1, "timestamp");
    
    EcDate date;
    EcBuffer buffer = ecbuf_create (200);
    
    ectime_getDate (&date);
    
    ecbuf_format (buffer, 200, "%04u-%02u-%02u %02u:%02u:%02u", date.year, date.month, date.day, date.hour, date.minute, date.sec);
    
    ecudc_setS (item, ecbuf_const_str (buffer));
    ecudc_add (data.content, &item);
    
    ecbuf_destroy (&buffer);
  }
  
  ecudc_add_asString(EC_ALLOC, data.content, "request-url", header->request_url);
  ecudc_add_asString(EC_ALLOC, data.content, "remote-address", header->remote_address);
  ecudc_add_asString(EC_ALLOC, data.content, "user-language", header->user_lang);
  ecudc_add_asString(EC_ALLOC, data.content, "user-agent", header->user_agent);

  if (ecstr_valid(header->title))
  {
    ecudc_add_asString(EC_ALLOC, data.content, "title", header->title);
  }
  
  ecmessages_broadcast (Q5_SERVICE_HTTP_REQUEST_INFO, &data, NULL);
    
  if ( isAssigned (data.content))
  {
    ecudc_destroy(EC_ALLOC, &(data.content));
  }  
}

//---------------------------------------------------------------------------------------

void echttp_header_dump (EcHttpHeader* header)
{
  eclogger_fmt (LL_TRACE, "ENTC", "http", "method:         %s", ecstr_cstring(header->method));
  eclogger_fmt (LL_TRACE, "ENTC", "http", "host:           %s", ecstr_cstring(header->host));
  eclogger_fmt (LL_TRACE, "ENTC", "http", "remote address: %s", ecstr_cstring(header->remote_address));
  eclogger_fmt (LL_TRACE, "ENTC", "http", "language:       %s", ecstr_cstring(header->session_lang));
  eclogger_fmt (LL_TRACE, "ENTC", "http", "url:            %s", ecstr_cstring(header->url));
  eclogger_fmt (LL_TRACE, "ENTC", "http", "request-url:    %s", ecstr_cstring(header->request_url));          
  eclogger_fmt (LL_TRACE, "ENTC", "http", "parameters:     %s", ecstr_cstring(header->request_params));
}

//---------------------------------------------------------------------------------------

struct EcHttpRequest_s
{
  
  EcString docroot;
  
  EcString tmproot;
  
  int header_on;
  
  EcHttpCallbacks callbacks;
  
};

//---------------------------------------------------------------------------------------

EcHttpRequest echttp_request_create (const EcString docroot, const EcString tmproot, int header)
{
  EcHttpRequest self = ENTC_NEW (struct EcHttpRequest_s);
  
  self->docroot = ecstr_copy (docroot);
  self->tmproot = ecstr_copy (tmproot);
  self->header_on = header;
  
  memset (&(self->callbacks), 0x0, sizeof(EcHttpCallbacks));
  
  return self;
}

//---------------------------------------------------------------------------------------

void echttp_request_destroy (EcHttpRequest* pself)
{
  EcHttpRequest self = *pself;

  ecstr_delete(&(self->docroot));
  ecstr_delete(&(self->tmproot));
  
  ENTC_DEL (pself, struct EcHttpRequest_s);
}

//---------------------------------------------------------------------------------------

EcUdc echttp_parse_auth (const EcString source)
{
  EcString auth_type;
  EcUdc auth = NULL;
  
  const EcString next_space = ecstr_pos (source, ' ');
  if (!ecstr_valid (next_space))
  {
    return NULL;
  }
  
  auth = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, "auth");

  auth_type = ecstr_part (source, next_space - source);

  ecudc_add_asString(EC_ALLOC, auth, "type", auth_type);

  ecudc_add_asString(EC_ALLOC, auth, "content", next_space + 1);
  
  eclogger_fmt (LL_TRACE, "ENTC", "http auth", "WWW-Authorization: %s %s", auth_type, next_space + 1);
  
  ecstr_delete (&auth_type);
  
  return auth;
}

//---------------------------------------------------------------------------------------

void echttp_parse_cookies_next (EcHttpHeader* header, const EcString cookie)
{
  const EcString res;
  //eclogger_logformat(self->logger, LOGMSG_INFO, "FCGI", "COOKIE '%s'", cookie );
  
  /* search for the quom4 session keyword */
  res = strstr (cookie, ecbuf_const_str (session_name));
  if(res)
  {
    // split into session part and language part
    ecstr_split( res + _ENTC_SESSION_NAMELENGTH + 1, &(header->sessionid), &(header->session_lang), '-');
  }
}

//---------------------------------------------------------------------------------------

void echttp_parse_cookies (EcHttpHeader* header, const EcString s)
{
  EcList list = eclist_create_ex (EC_ALLOC);
  EcListNode node;
  
  ecstr_tokenizer(list, s, ';');
  
  for(node = eclist_first(list); node != eclist_end(list); node = eclist_next(node))
  {
    EcString token = eclist_data(node);
    echttp_parse_cookies_next (header, token);
    ecstr_delete( &token );
  }
  
  eclist_free_ex (EC_ALLOC, &list);
}

//---------------------------------------------------------------------------------------

void echttp_parse_lang (EcHttpHeader* header, const EcString s)
{
  EcList list = eclist_create_ex (EC_ALLOC);
  EcListNode node;
  
  ecstr_tokenizer(list, s, ';');
  
  node = eclist_first(list);
  if (node != eclist_end(list))
  {
    
  }
  
  ecstr_tokenizer_clear(list);  
  eclist_free_ex (EC_ALLOC, &list);
}

//---------------------------------------------------------------------------------------

int echttp_parse_header (EcHttpHeader* header, EcStreamBuffer buffer)
{
  int error;
  EcStream stream = ecstream_new();
  
  char b1 = 0;
  char b2 = 0;
  
  while (ecstreambuffer_readln (buffer, stream, &error, &b1, &b2))
  {      
    const char* line = ecstream_buffer(stream);
    if( *line )
    {
      //eclogger_msg(LL_TRACE, "ENTC", "http header", line);
      
      /* Host: 127.0.0.1:8080 */
      if( line[0] == 'H' && line[1] == 'o' && line[5] == ' ' )        
      {
        ecstr_replace( &(header->host), line + 6);
      }
      /* Cookie: _QUOM_SESSION=oaemjysdlhapguwgcmsakuoszbviykjc */
      else if( line[0] == 'C' && line[1] == 'o' && line[7] == ' ' )
      {
        echttp_parse_cookies (header, line + 7);
      }
      // Accept-Language: en-US,en;q=0.8
      else if( line[0] == 'A' && line[1] == 'c' && line[16] == ' ')
      {
        echttp_parse_lang (header, line + 16);
      }
      // User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.107 Safari/537.36
      else if ((line[0] == 'U')&&(line[4] == '-')&&(line[9] == 't'))
      {
        ecstr_replace(&(header->user_agent), line + 12);
      }
      // Content-Length: 27
      else if ((line[0] == 'C')&&(line[7] == '-')&&(line[9] == 'e'))
      {
        header->content_length = atoi(line + 16);
      }
      // Authorization: xxx
      else if ((line[0] == 'A')&&(line[7] == 'z')&&(line[14] == ' '))
      {
        header->auth = echttp_parse_auth (line + 15);
      }
      // Expect: 100-continue
      else if ((line[0] == 'E')&&(line[6] == ':'))
      {
        header->header_only = 100;
      }
      else
      {
        // add special header value to map
        EcString key = ecstr_init ();
        EcString val = ecstr_init ();
        
        if (ecstr_split(line, &key, &val, ':'))
        {
          ecstr_replaceTO (&key, ecstr_trim (key));
          ecstr_replaceTO (&val, ecstr_trim (val));
          
          ecstr_toUpper(key);
          
          ecmapchar_append(header->values, key, val);
        }

        ecstr_delete (&key);
        ecstr_delete (&val);
      }
    }
    else
    {
      //printf("{recv end}\n");
      break;        
    }
  }
  
  ecstream_delete(&stream);
  
  return TRUE;
}

//---------------------------------------------------------------------------------------

int echttp_parse_method (EcHttpHeader* header, EcStreamBuffer buffer, EcStream streambuffer)
{
  int error;
  
  char b1 = 0;
  char b2 = 0;

  // parse the first line received
  if ( ecstreambuffer_readln (buffer, streambuffer, &error, &b1, &b2) )
  {
    const char* line = ecstream_buffer(streambuffer);
    if(!ecstr_empty(line))
    {
      const EcString after_method;
      const EcString afetr_url;

      // eclogger_fmt (LL_TRACE, "ENTC", "http", "{first line} '%s'", line);
      // find method
      after_method = ecstr_pos (line, ' ');
      if (ecstr_empty (after_method))
      {
        return FALSE;
      }
      
      header->method = ecstr_part(line, after_method - line);
      
      after_method++;
      // find url
      afetr_url = ecstr_pos (after_method, ' ');
      if (ecstr_empty (afetr_url))
      {
        return FALSE;
      }
      
      header->url = ecstr_part (after_method, afetr_url - after_method);         
    }
  }
  else
  {
    // check error and send response
    if (error == ENTC_SOCKET_RETSTATE_TIMEOUT)
    {
      return FALSE;
    }
  }
  // if not method found, we cannot handle the request
  return ecstr_valid (header->method);
}

//---------------------------------------------------------------------------------------

int echttp_request_next (EcHttpHeader* header, EcStreamBuffer buffer, EcStream streambuffer)
{
  int ret = TRUE;
  // parse incoming stream of 'GET/POST/...'
  ret = ret && echttp_parse_method (header, buffer, streambuffer);
  // parse meta informations
  ret = ret && echttp_parse_header (header, buffer);
    
  return ret;
}

//---------------------------------------------------------------------------------------

void echttp_request_clear (EcHttpRequest self, void* ptr, void** pobject)
{
  if (isAssigned (self->callbacks.clear))
  {
    self->callbacks.clear (ptr, pobject);
  }
}

//---------------------------------------------------------------------------------------

void* echttp_request_flow_stream (EcHttpRequest self, EcHttpHeader* header, EcStreamBuffer buffer, EcDevStream stream, EcSocket socket)
{
  void* object = NULL;

  // get a route
  if (!self->callbacks.route (self->callbacks.ptr, header, &object))
  {
    echttp_send_file (header, socket, self->docroot);
    return object;
  }
  
  if (!self->callbacks.validate (self->callbacks.ptr, header, stream, &object))
  {
    return object;
  }
  
  if (header->header_only > 0)
  {
    if (header->header_only == 100)
    {
      // just send this header
      eclogger_msg (LL_TRACE, "ENTC", "http process", "seen 100-continue, response sent");
      echttp_send_100 (header, socket);
    }
    else
    {
      echttp_send_417 (header, socket);
      return object;      
    }
  }
  
  // get POST content
  if (header->content_length > 0)
  {
    // eclogger_fmt (LL_TRACE, "ENTC", "http process", "request has content: %i", header->content_length);
    
    if (isAssigned (header->content))
    {
      echttp_content_destroy (&(header->content));
    }
    
    header->content = echttp_content_create (header->content_length, echttp_content_callback_bf, echttp_content_callback_mm, buffer, self->tmproot);  
    if (isNotAssigned (header->content)) 
    {
      echttp_send_500 (header, socket);
      return object;
    }
  }
  else
  {
    // eclogger_msg (LL_TRACE, "ENTC", "http process", "no content");    
  }
  
  // render
  self->callbacks.render (self->callbacks.ptr, header, stream, &object);
  
  // log visit
  echttp_header_lotToService (header); 
  
  return object;
}

//---------------------------------------------------------------------------------------

void echttp_request_flow (EcHttpRequest self, EcHttpHeader* header, EcStreamBuffer buffer, EcStream streambuffer, EcSocket socket)
{
  // check first if we handle this in a custom way
  if (isNotAssigned (self->callbacks.route))
  {
    echttp_send_file (header, socket, self->docroot);
    return;
  }
  
  if (isNotAssigned (self->callbacks.validate))
  {
    echttp_send_file (header, socket, self->docroot);  
    return;    
  }  
  
  if (isNotAssigned (self->callbacks.render))
  {
    echttp_send_file (header, socket, self->docroot);  
    return;    
  }  

  // fill header
  if (!echttp_request_next (header, buffer, streambuffer))
  {
    echttp_send_408 (header, socket); 
    return;
  }
  
  // **** validate header ****
  echttp_header_validate (header);
  
  echttp_header_title (header);

  {    
    EcDevStream stream = ecdevstream_new (1024, q4http_callback, socket); // output stream

    void* object = echttp_request_flow_stream (self, header, buffer, stream, socket);
  
    echttp_request_clear (self, self->callbacks.ptr, &object);
  
    ecdevstream_delete (&stream);  
  }
}

//---------------------------------------------------------------------------------------

void* echttp_request_dev_stream (EcHttpRequest self, EcHttpHeader* header, EcDevStream stream, void* callback_ptr)
{
  void* object = NULL;
  
  // get a route
  if (!self->callbacks.route (callback_ptr, header, &object))
  {
    return object;
  }
  
  if (!self->callbacks.validate (callback_ptr, header, stream, &object))
  {
    return object;
  }  
  
  // retrieve POST content
  if (isAssigned (self->callbacks.content))
  {
    self->callbacks.content (callback_ptr, header, self->tmproot);
  }
  
  // render
  self->callbacks.render (callback_ptr, header, stream, &object);
  
  // log visit
  echttp_header_lotToService (header);
  
  return object;
}

//---------------------------------------------------------------------------------------

void echttp_request_dev_flow (EcHttpRequest self, EcHttpHeader* header, EcDevStream stream, void* callback_ptr)
{
  // check first if we handle this in a custom way
  if (isNotAssigned (self->callbacks.route))
  {
    eclogger_msg (LL_WARN, "ENTC", "http", "no route callback is set");      
    return;
  }
  
  if (isNotAssigned (self->callbacks.validate))
  {
    eclogger_msg (LL_WARN, "ENTC", "http", "no validate callback is set");      
    return;    
  }  
  
  if (isNotAssigned (self->callbacks.render))
  {
    eclogger_msg (LL_WARN, "ENTC", "http", "no render callback is set");      
    return;    
  }  
  
  if (isAssigned (self->callbacks.custom_header))
  {
    self->callbacks.custom_header (callback_ptr, header);
  }
  else
  {
    eclogger_msg (LL_WARN, "ENTC", "http", "no header callback is set"); 
  }

  // **** validate header ****
  echttp_header_validate (header);
  
  echttp_header_title (header);

  {
    void* object = echttp_request_dev_stream (self, header, stream, callback_ptr);  
  
    echttp_request_clear (self, self->callbacks.ptr, &object);
  }
}

//---------------------------------------------------------------------------------------

void echttp_request_process_dev (EcHttpRequest self, EcDevStream stream, void* callback_ptr)
{
  EcHttpHeader header;

  // initialize the header struct
  echttp_header_init (&header, self->header_on);

  echttp_request_dev_flow (self, &header, stream, callback_ptr);
  
  echttp_header_clear (&header);
}

//---------------------------------------------------------------------------------------

void echttp_request_process (EcHttpRequest self, EcSocket socket)
{
  EcHttpHeader header;
  
  EcStreamBuffer buffer = ecstreambuffer_create (socket);
  EcStream streambuffer = ecstream_new ();

  // initialize the header struct
  echttp_header_init (&header, self->header_on);

  echttp_request_flow (self, &header, buffer, streambuffer, socket);
  
  echttp_header_clear (&header);
  
  ecstreambuffer_destroy (&buffer);
  ecstream_delete (&streambuffer);
}

//---------------------------------------------------------------------------------------

void echttp_request_callbacks (EcHttpRequest self, EcHttpCallbacks* callbacks)
{
  memcpy (&(self->callbacks), callbacks, sizeof(EcHttpCallbacks));
}

//---------------------------------------------------------------------------------------

EcUdc echttp_getParams (EcHttpHeader* header)
{
  // check for token params
  if (ecstr_valid (header->request_params))
  {
    EcUdc ret = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
    
    EcListCursor cursor;
    EcList tokens = eclist_create ();
    
    ecstr_tokenizer(tokens, header->request_params, '&');
    
    eclist_cursor (tokens, &cursor);
    while (eclist_cnext (&cursor))
    {
      EcString key = ecstr_init ();
      EcString val = ecstr_init ();
      
      if (ecstr_split (cursor.value, &key, &val, '='))
      {
        EcString value;

        echttp_unescape (val);
        
        value = ecstr_trimc (val, '"');
        
        ecudc_add_asS_o (EC_ALLOC, ret, key, &value);
      }
      
      ecstr_delete (&key);
      ecstr_delete (&val);
    }
    
    return ret;
  }

  return NULL;
}

//---------------------------------------------------------------------------------------
