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

#include <utils/ecstreambuffer.h>
#include <utils/ecreadbuffer.h>
#include <utils/ecsecfile.h>
#include <types/ecmapchar.h>
#include <string.h>
#include <system/ectime.h>

static EcMapChar mime_types = NULL;
EcBuffer session_name;

#define _ENTC_SESSION_NAMELENGTH 23

static int echhtp_counter = 0;

//---------------------------------------------------------------------------------------

void echttp_init (void)
{
  if (echhtp_counter == 0)
  {
    session_name = ecstr_buffer(_ENTC_SESSION_NAMELENGTH);  
    ecstr_random(session_name, _ENTC_SESSION_NAMELENGTH);
    
    mime_types = ecmapchar_new(); 
    
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
    ecmapchar_append( mime_types, "css",      "text/css" );
    ecmapchar_append( mime_types, "html",     "text/html" );
    ecmapchar_append( mime_types, "htm",      "text/html" );
    ecmapchar_append( mime_types, "js",       "text/javascript" );
    ecmapchar_append( mime_types, "asc",      "text/plain" );
    ecmapchar_append( mime_types, "c",        "text/plain" ); 
    ecmapchar_append( mime_types, "ico",      "image/x-ico; charset=binary" );
    
    ecmapchar_append( mime_types, "exe",      "application/octet-stream" );     
  }
  echhtp_counter++;
}

//---------------------------------------------------------------------------------------

void echttp_done (void)
{
  echhtp_counter--;
  if (echhtp_counter == 0) 
  {
    ecmapchar_delete(&mime_types);
    ecstr_release(&session_name);
  }
}

//---------------------------------------------------------------------------------------

const EcString echttp_getMimeType (const EcString filename, EcLogger logger)
{
  /* variables */
  const EcString pos01;
  const EcString pos02;
  EcMapCharNode node;
  
  pos01 = ecfs_extractFile(filename);
  
  if( !pos01 )
  {
    eclogger_logformat(logger, LL_WARN, "SERV", "{file} can't extract filename from '%s'", filename);
    
    return "application/octet-stream";
  }
  
  /* extract the extension from the file */
  pos02 = strrchr(pos01, '.');
  
  if( !pos02 )
  {
    eclogger_logformat(logger, LL_WARN, "SERV", "{file} can't find file extension from '%s'", filename);
    
    return "application/octet-stream";
  }
  
  node = ecmapchar_find( mime_types, pos02 + 1 );
  if( node != ecmapchar_end( mime_types ))
  {
    return ecmapchar_data( node );  
  }
  else
  {
    eclogger_logformat(logger, LL_WARN, "SERV", "{file} can't find mime type '%s' from '%s'", pos02 + 1, filename);
    return "application/octet-stream";
  }   
}

//---------------------------------------------------------------------------------------

void echttp_send_startHeaderOK (EcHttpHeader* header, EcDevStream stream)
{
  if (header->header_on)
  {
    ecdevstream_appends( stream, "HTTP/1.1 200 OK\r\n" );
    ecdevstream_appends( stream, "Server: " );
    ecdevstream_appends( stream, "entc" );
    ecdevstream_appends( stream, "\r\n" );
  }
  ecdevstream_appends( stream, "Content-type: ");
  ecdevstream_appends( stream, header->mime);
  ecdevstream_appends( stream, "\r\n" );    
  
  // write cookies
  if(ecstr_valid(header->sessionid))
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
  
  ecdevstream_appends( stream, "Content-Language: ");
  ecdevstream_appends( stream, header->session_lang );
  ecdevstream_appends( stream, "\r\n" );
}

//---------------------------------------------------------------------------------------

void echttp_send_finishHeader(EcHttpHeader* header, EcDevStream stream)
{
  ecdevstream_append( stream, "\r\n", 2 );
}

//---------------------------------------------------------------------------------------

void echttp_send_DefaultHeader (EcHttpHeader* header, EcDevStream stream)
{
  echttp_send_startHeaderOK (header, stream);  
  echttp_send_finishHeader (header, stream);  
}

//---------------------------------------------------------------------------------------

void q4http_callback(void* ptr, const void* buffer, uint_t nbyte)
{
  ecsocket_write(ptr, buffer, nbyte);
}

//---------------------------------------------------------------------------------------

void echttp_send_408Timeout (EcHttpHeader* header, EcSocket socket)
{  
  if (header->header_on)
  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket); // output stream
    // send back that the file doesn't exists
    ecdevstream_appends( stream, "HTTP/1.1 408 Request Timeout\r\n" );
    ecdevstream_appends( stream, "\r\n\r\n" );

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
  echttp_send_DefaultHeader (header, stream);
  
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

void echttp_send_file (EcHttpHeader* header, EcSocket socket, const EcString docroot, EcLogger logger)
{
  // variables
  EcString filename = ecstr_copy (header->request_url);
  EcString path;
  struct EcSecFopen secopen;
  int res;
  
  if (ENTC_PATH_SEPARATOR != '/')
  {
    //ecstr_replaceAllChars(filename, '/', Ec_PATH_SEPARATOR);
  }
  
  eclogger_logformat(logger, LL_TRACE, "SERV", "try to send file '%s'", filename);

  path = ecfs_mergeToPath(docroot, filename);
  
  //eclogger_logformat(logger, LOGMSG_INFO, "SERV", "try to access '%s'", path);

  // try to open filename
  res = ecsec_fopen(&secopen, path, O_RDONLY, logger, docroot);
  // cleanup
  ecstr_delete(&path); 
  
  if (res == FALSE) 
  {
    ecstr_delete(&filename);
    ecstr_delete(&(secopen.filename));
    
    if (secopen.sec_error != 0) 
    {
      echttp_send_SecureIncident (header, socket);
      return;
    }
    echttp_send_404NotFound (header, socket, ecstr_init());    
    return;
  }

  header->mime = echttp_getMimeType(filename, logger);
  
  //eclogger_logformat(logger, LOGMSG_INFO, "SERV", "send file '%s' : '%s'", filename, hr->mime );
  
  ecstr_delete(&filename); 

  {
    EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket);  // output stream for header
    
    echttp_send_DefaultHeader (header, stream);

    ecdevstream_delete( &stream ); 
    
    ecsocket_writeFile(socket, secopen.fhandle);
    
    ecfh_close( &(secopen.fhandle) );  
  }
  
  ecstr_delete(&(secopen.filename));
}

//---------------------------------------------------------------------------------------

void echttp_header_init (EcHttpHeader* header, int header_on)
{
  header->method = C_REQUEST_METHOD_INVALID;
  header->header_on = header_on;
  header->host = ecstr_init();
  header->remote_address = ecstr_init();
  header->user_lang = ecstr_init();
  header->user_agent = ecstr_init();
  header->sessionid = ecstr_init();
  header->session_lang = ecstr_init();
  header->url = NULL;
  header->request_url = ecstr_init();
  header->request_params = ecstr_init(); 
  header->mime = ecstr_init();
  header->title = ecstr_init();
  header->tokens = NULL;
  header->token = NULL;
  header->urlpath = ecstr_init();
}

//---------------------------------------------------------------------------------------

void echttp_header_clear (EcHttpHeader* header)
{
  ecstr_delete(&(header->host));
  ecstr_delete(&(header->user_lang));
  ecstr_delete(&(header->user_agent));
  ecstr_delete(&(header->sessionid));
  ecstr_delete(&(header->session_lang));
  ecstr_delete(&(header->request_params));
  ecstr_delete(&(header->request_url));
  ecstr_delete(&(header->title));
  
  if (isAssigned (header->tokens))
  {
    ecstr_tokenizer_clear(header->tokens);
    eclist_delete(&(header->tokens));
  }  
  ecstr_delete(&(header->urlpath));
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
  
  header->tokens = eclist_new();
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
    header->title = ecstr_trans(&buffer);
  }
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
  }
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
}

//---------------------------------------------------------------------------------------

void echttp_header_lotToService (EcHttpHeader* header, EcLogger logger)
{
  // generate the udc containers with all the infos we have :-)
  EcUdc udc = ecudc_new(0, "AccessInfo");
  
  {
    EcUdc item = ecudc_new(1, "timestamp");
    
    EcDate date;
    EcBuffer buffer = ecstr_buffer(200);
    
    ectime_getDate (&date);
    
    ecstr_format (buffer, 200, "%04u-%02u-%02u %02u:%02u:%02u", date.year, date.month, date.day, date.hour, date.minute, date.sec);
    
    ecudc_setS(item, ecstr_get(buffer));
    ecudc_add(udc, &item);
    
    ecstr_release(&buffer);
  }
  {
    EcUdc item = ecudc_new(1, "request-url");
    ecudc_setS(item, header->request_url);
    ecudc_add(udc, &item);
  }
  {
    EcUdc item = ecudc_new(1, "remote-address");
    ecudc_setS(item, header->remote_address);
    ecudc_add(udc, &item);
  }
  {
    EcUdc item = ecudc_new(1, "user-language");
    ecudc_setS(item, header->user_lang);
    ecudc_add(udc, &item);
  }
  {
    EcUdc item = ecudc_new(1, "user-agent");
    ecudc_setS(item, header->user_agent);
    ecudc_add(udc, &item);
  }
  if (ecstr_valid(header->title))
  {
    EcUdc item = ecudc_new(1, "title");
    ecudc_setS(item, header->title);
    ecudc_add(udc, &item);
  }
  
  eclogger_message(logger, 10000, 1, &udc);
  
  if ( isAssigned (udc))
  {
    ecudc_del(&udc);
  }  
}

//---------------------------------------------------------------------------------------

void echttp_header_dump (EcHttpHeader* header, EcLogger logger)
{
  static char* method_matrix[5] = {"INVALID", "GET", "PUT", "POST", "DELETE"};

  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} method:         %s", method_matrix[header->method]);
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} host:           %s", ecstr_cstring(header->host));
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} remote address: %s", ecstr_cstring(header->remote_address));
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} sessionid:      %s", ecstr_cstring(header->sessionid));
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} language:       %s", ecstr_cstring(header->session_lang));
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} url:            %s", ecstr_cstring(header->url));
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} request-url:    %s", ecstr_cstring(header->request_url));          
  eclogger_logformat(logger, LL_DEBUG, "SERV", "{request} parameters:     %s", ecstr_cstring(header->request_params));
}

//---------------------------------------------------------------------------------------

struct EcHttpRequest_s
{
  
  EcLogger logger;
  
  EcString docroot;
  
  int header_on;
  
  EcHttpCallbacks callbacks;
  
};

//---------------------------------------------------------------------------------------

EcHttpRequest echttp_request_create (const EcString docroot, int header, EcLogger logger)
{
  EcHttpRequest self = ENTC_NEW (struct EcHttpRequest_s);
  
  self->logger = logger;
  self->docroot = ecstr_copy (docroot);
  self->header_on = header;
  
  memset (&(self->callbacks), 0x0, sizeof(EcHttpCallbacks));
  
  return self;
}

//---------------------------------------------------------------------------------------

void echttp_request_destroy (EcHttpRequest* pself)
{
  EcHttpRequest self = *pself;

  ecstr_delete(&(self->docroot));
  
  ENTC_DEL (pself, struct EcHttpRequest_s);
}

//---------------------------------------------------------------------------------------

void echttp_parse_cookies_next (EcHttpHeader* header, const EcString cookie)
{
  const EcString res;
  //eclogger_logformat(self->logger, LOGMSG_INFO, "FCGI", "COOKIE '%s'", cookie );
  
  /* search for the quom4 session keyword */
  res = strstr(cookie, ecstr_get(session_name));
  if(res)
  {
    // split into session part and language part
    ecstr_split( res + _ENTC_SESSION_NAMELENGTH + 1, &(header->sessionid), &(header->session_lang), '-');
  }
}

//---------------------------------------------------------------------------------------

void echttp_parse_cookies (EcHttpHeader* header, const EcString s)
{
  EcList list = eclist_new();
  EcListNode node;
  
  ecstr_tokenizer(list, s, ';');
  
  for(node = eclist_first(list); node != eclist_end(list); node = eclist_next(node))
  {
    EcString token = eclist_data(node);
    echttp_parse_cookies_next (header, token);
    ecstr_delete( &token );
  }
  
  eclist_delete( &list );
}

//---------------------------------------------------------------------------------------

void echttp_parse_lang (EcHttpHeader* header, const EcString s)
{
  EcList list = eclist_new();
  EcListNode node;
  
  ecstr_tokenizer(list, s, ';');
  
  node = eclist_first(list);
  if (node != eclist_end(list))
  {
    
  }
  
  ecstr_tokenizer_clear(list);  
  eclist_delete( &list );
}

//---------------------------------------------------------------------------------------

int echttp_parse_meta (EcHttpHeader* header, EcStreamBuffer buffer, EcLogger logger)
{
  int error;
  EcStream stream = ecstream_new();
  
  while( ecstreambuffer_readln(buffer, stream, &error) )
  {      
    const char* line = ecstream_buffer(stream);
    if( *line )
    {
      //eclogger_logformat(logger, LL_TRACE, "SERV", "{recv line} '%s'", line);
      
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
      // 
    }
    else
    {
      //eclogger_logformat(qo->logger, LOGMSG_INFO, "SERV", "{recv end} '%s'", line);
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
  // parse the first line received
  if ( ecstreambuffer_readln (buffer, streambuffer, &error) )
  {
    const char* line = ecstream_buffer(streambuffer);
    if(!ecstr_empty(line))
    {
      int lenline = strlen(line);
      char* pos;
      
      if ((lenline > 3)&&(line[0] == 'G')&&(line[3] == ' '))
      {
        // GET
        if (lenline > 4)
        {
          pos = strrchr(line + 4, ' ');
          *pos = 0;
          
          header->url = line + 4;          
        }
        else
        {
          header->url = NULL;
        }
        header->method = C_REQUEST_METHOD_GET;
      } 
      else if ((lenline > 3)&&(line[0] == 'P')&&(line[3] == ' '))
      {
        // PUT
        pos = strrchr(line + 4, ' ');
        *pos = 0;
        
        header->url = line + 4;
        header->method = C_REQUEST_METHOD_PUT;
      }
      else if ((lenline > 4)&&(line[0] == 'P')&&(line[4] == ' '))
      {
        // POST
        pos = strrchr(line + 5, ' ');
        *pos = 0;
        
        header->url = line + 5;
        header->method = C_REQUEST_METHOD_POST;
      }
      else if ((lenline > 6)&&(line[0] == 'D')&&(line[6] == ' '))
      {
        // DELETE
        pos = strrchr(line + 7, ' ');
        *pos = 0;
        
        header->url = line + 7;
        header->method = C_REQUEST_METHOD_DELETE;
      }
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
  return header->method != C_REQUEST_METHOD_INVALID;
}

//---------------------------------------------------------------------------------------

int echttp_request_next (EcHttpHeader* header, EcStreamBuffer buffer, EcStream streambuffer, EcLogger logger)
{
  int ret = TRUE;
  // parse incoming stream of 'GET/POST/...'
  ret = ret && echttp_parse_method (header, buffer, streambuffer);
  
  // parse meta informations
  ret = ret && echttp_parse_meta (header, buffer, logger);
  
  return ret;
}

//---------------------------------------------------------------------------------------

void echttp_request_process_dev (EcHttpRequest self, EcDevStream stream, http_header_fct fct, void* ptr, EcLogger logger)
{
  EcHttpHeader header;

  // initialize the header struct
  echttp_header_init (&header, self->header_on);

  // check first if we handle this in a custom way
  if (isAssigned (self->callbacks.process))
  {
    void* object = NULL;
    
    if (fct)
    {
      fct (ptr, &header);
    }

    echttp_header_validate (&header);
    
    echttp_header_title (&header);

    if (self->callbacks.process (self->callbacks.process_ptr, &header, &object))
    {
      echttp_send_DefaultHeader (&header, stream);
      
      if (isAssigned (self->callbacks.render))
      {
        self->callbacks.render (self->callbacks.render_ptr, &header, stream, &object);
      }
      else
      {
        ecdevstream_appends(stream, "no render");
      }      
    }
  }
  
  echttp_header_clear (&header);
}

//---------------------------------------------------------------------------------------

void echttp_request_process_next (EcHttpRequest self, EcHttpHeader* header, EcSocket socket, EcLogger logger)
{
  // check first if we handle this in a custom way
  if (isAssigned (self->callbacks.process))
  {
    void* object = NULL;
    
    echttp_header_title (header);
    
    if (self->callbacks.process (self->callbacks.process_ptr, header, &object))
    {
      EcDevStream stream = ecdevstream_new(1024, q4http_callback, socket); // output stream
      
      echttp_send_DefaultHeader (header, stream);
      
      if (isAssigned (self->callbacks.render))
      {
        self->callbacks.render (self->callbacks.render_ptr, header, stream, &object);
      }
      else
      {
        ecdevstream_appends(stream, "no render");
      }
      
      ecdevstream_delete(&stream);
      
      return;
    }
  }
  echttp_send_file (header, socket, self->docroot, logger);  
}

//---------------------------------------------------------------------------------------

void echttp_request_process (EcHttpRequest self, EcSocket socket, EcLogger logger)
{
  EcHttpHeader header;
  
  EcStreamBuffer buffer = ecstreambuffer_new(logger, socket);
  EcStream streambuffer = ecstream_new ();

  // initialize the header struct
  echttp_header_init (&header, self->header_on);

  if (echttp_request_next (&header, buffer, streambuffer, logger))
  {
    echttp_header_validate (&header);
    
    echttp_request_process_next (self, &header, socket, logger);
  }
  else
  {
    echttp_send_408Timeout (&header, socket); 
  }
  
  echttp_header_clear (&header);
  
  ecstreambuffer_delete (&buffer);
  ecstream_delete (&streambuffer);
}

//---------------------------------------------------------------------------------------

void echttp_request_callbacks (EcHttpRequest self, EcHttpCallbacks* callbacks)
{
  memcpy (&(self->callbacks), callbacks, sizeof(EcHttpCallbacks));
}

//---------------------------------------------------------------------------------------
