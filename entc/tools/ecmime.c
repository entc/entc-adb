/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "ecmime.h"

#include "types/ecbuffer.h"
#include "types/ecmapchar.h"
#include "tools/ectokenizer.h"

//------------------------------------------------------------------------------------------------------

const EcString ecmime_getFromFile (const EcString filename)
{
  const EcString ext = ecfs_extractFileExtension (filename);
  
  if (ext)
  {
    return ecmime_getFromExtension (ext);
  }
  else
  {
    eclogger_fmt (LL_WARN, "ENTC", "mime", "can't extract extension from '%s'", filename);
    return "application/octet-stream";
  }
}

//------------------------------------------------------------------------------------------------------

const EcString ecmime_getFromExtension (const EcString ext)
{
  static EcMapChar mime_types = NULL;
  
  if (mime_types == NULL)
  {
    eclogger_fmt (LL_TRACE, "ENTC", "mime", "create new mime types");

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
    ecmapchar_append( mime_types, "txt",      "text/plain" );
    ecmapchar_append( mime_types, "json",     "application/json");
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
  
  {
    EcMapCharNode node = ecmapchar_find( mime_types, ext);
    
    if( node != ecmapchar_end( mime_types ))
    {
      return ecmapchar_data( node );
    }
    else
    {
      eclogger_fmt (LL_WARN, "ENTC", "mime", "can't find mime type from extension '%s'", ext);
      return "application/octet-stream";
    }
  }
}

//------------------------------------------------------------------------------------------------------

#define C_MAX_BUFSIZE 100

struct EcMultipartParser_s
{
  
  EcString boundary;
  
  ulong_t boundaryLenMin;

  ulong_t boundaryLenMax;

  http_content_callback cb;
  
  void* ptr;
  
  EcBuffer buffer;
  
  EcStream line;
  
  int state;
  
  int breakType;
  
  int state2;
  
  EcString path;
  
  EcHttpContent hc;
  
  // binary data direct to file
  
  EcStream content;
  
  int addLineBreak;
  
  EcDevStream devsteam;
  
  EcFileHandle fh; 
  
  EcString fn;
  
  EcMapChar params;
  
  EcFileHandle fhDebug;
  
  // object callback
  
  ecmultipartparser_callback obj_dc;
  
  void* obj_ptr;
  
};

typedef struct
{

  int state;
  
  ulong_t read;
  ulong_t size;
  
  ulong_t bpos;
  ulong_t bsiz;
  
  const char* data;
  
  const char* b1;
  const char* b2;

} EcStateParser;

typedef struct
{
  
  const char* pos;
  
  ulong_t len;
  
  EcString name;
  
} EcParserData;

//-----------------------------------------------------------------------------------------------------------

EcMultipartParser ecmultipartparser_create (const EcString boundary, const EcString path, http_content_callback cb, void* ptr, EcHttpContent hc, ecmultipartparser_callback dc, void* obj)
{
  EcMultipartParser self = ENTC_NEW (struct EcMultipartParser_s);
  
  self->boundary = ecstr_cat2 ("--", boundary);
  self->boundaryLenMin = ecstr_len (self->boundary);
  self->boundaryLenMax = self->boundaryLenMin + 4;
  
  self->cb =cb;
  self->ptr = ptr;
  self->hc = hc;
  
  self->path = ecstr_copy(path);
  
  self->buffer = ecbuf_create (C_MAX_BUFSIZE);
  self->line = ecstream_create ();
  self->params = ecmapchar_create (EC_ALLOC); 
  
  self->state = 0;
  self->breakType = 0;

  self->content = NULL;
  self->addLineBreak = FALSE;
  self->devsteam = NULL;
  self->fn = NULL;
  
  self->obj_dc = dc;
  self->obj_ptr = obj;
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_destroy (EcMultipartParser* pself)
{
  EcMultipartParser self = *pself;
  
  ecstr_delete(&(self->path));
  
  ecstr_delete (&(self->boundary));
  ecbuf_destroy (&(self->buffer));
  ecstream_destroy (&(self->line));
  
  if (self->params)
  {
    ecmapchar_destroy(EC_ALLOC, &(self->params));
  }
  
  if (self->content)
  {
    ecstream_destroy (&(self->content));
  }
  
  ENTC_DEL (pself, struct EcMultipartParser_s);
}

//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_write (void* ptr, const void* buffer, uint_t nbyte)
{
  EcMultipartParser self = ptr;
  
  ecfh_writeConst (self->fh, buffer, nbyte);
}

//-----------------------------------------------------------------------------------------------------------

void echttpheader_parseParam (EcMapChar map, const EcString line)
{
  // add special header value to map
  EcString key = ecstr_init ();
  EcString val = ecstr_init ();
  
  if (ecstr_split(line, &key, &val, ':'))
  {
    ecstr_replaceTO (&key, ecstr_trim (key));
    ecstr_replaceTO (&val, ecstr_trim (val));
    
    ecstr_toUpper(key);
    
    ecmapchar_append (map, key, val);
    
    //eclogger_fmt (LL_TRACE, "ENTC", "mime", "add param '%s':'%s'", key, val);
  }
  
  ecstr_delete (&key);
  ecstr_delete (&val);
}

//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_isBoundary (EcMultipartParser self, const EcString line, ulong_t lsize, int addToContent)
{
  /*
  if (lsize < self->boundaryLenMax && lsize >= self->boundaryLenMin)
  {
    eclogger_fmt (LL_TRACE, "ENTC", "mime", "check line '%s' len: %u size: %u", line, lsize); 
  }
   */
  
  // reached boundary
  if ((lsize < self->boundaryLenMax) && (lsize >= self->boundaryLenMin) && ecstr_leading (line, self->boundary))
  {
    // create a new http content
    if (self->obj_dc)
    {
      // transform stream to buffer
      EcBuffer buf = ecstream_tobuf (&(self->content));

      self->obj_dc (self->obj_ptr, &buf, &(self->params));

      // recreate an empty map
      self->params = ecmapchar_create (EC_ALLOC);
    }
    else if (isAssigned (self->hc))
    {
      // transform stream to buffer
      EcBuffer buf = ecstream_tobuf (&(self->content));
      
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "add content '%u'", buf->size); 
      
      // create a new content part
      EcHttpContent hc = echttp_content_create2 (&buf, &(self->params));
      
      // assign the new content as next content
      self->hc = echttp_content_add (self->hc, &hc);
      
      // recreate an empty map
      self->params = ecmapchar_create (EC_ALLOC); 
    }
    else
    {
      eclogger_fmt (LL_WARN, "ENTC", "mime", "no hc, content deleted"); 

      ecstream_destroy (&(self->content));
    }
    
    self->addLineBreak = FALSE;    
  }
  else if (addToContent)
  {
    if (self->addLineBreak)
    {
      switch (self->breakType)
      {
        case 1: ecstream_append_c (self->content, '\r'); break;
        case 2: ecstream_append_c (self->content, '\n'); break;
        case 3:
        {
          ecstream_append_c (self->content, '\r');
          ecstream_append_c (self->content, '\n');
        }
          break;
      }
      
      self->addLineBreak = FALSE;
    }
    
    // append to content
    ecstream_append_stream (self->content, self->line);
    
    self->addLineBreak = TRUE;        
  }
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_line (EcMultipartParser self, EcParserData* pd)
{  
  // get last line
  const EcString line = ecstream_get (self->line);

  if (self->content)
  {
    ecmultipartparser_isBoundary (self, line, ecstream_size (self->line), TRUE);
  }
  else if (line [0] == 0)
  {
    //eclogger_fmt (LL_TRACE, "ENTC", "mime", "switch to content"); 

    self->content = ecstream_create ();
  }
  else
  {
    // add to parameters
    echttpheader_parseParam (self->params, line);    
  }
  
  //printf("\n======================================\n%s\n======================================[%u]\n", line, ecstream_size (self->line));
  
  //eclogger_fmt (LL_TRACE, "ENTC", "mime", "clear line"); 
  ecstream_clear (self->line);

  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state0 (EcMultipartParser self, EcParserData* pd)
{
  // initial line
  // find what kind of line breaks we have and parse
  const char* c;
  for (c = pd->pos; (pd->len > 0); c++)
  {
    (pd->len)--;
    
    if (*c == '\r')
    {
      if (self->breakType > 0)
      {
        eclogger_msg (LL_WARN, "ENTC", "mime", "wrong sequence of breaks #1"); 
        return FALSE;
      }
      
      self->breakType = 1;
      continue;
    }
    
    if (*c == '\n')
    {
      if (self->breakType == 0)
      {
        self->breakType = 2;
        continue;
      }
      else if (self->breakType == 1)
      {
        self->breakType = 3;
        continue;
      }
      else
      {
        eclogger_msg (LL_WARN, "ENTC", "mime", "wrong sequence of breaks #2"); 
        return FALSE;
      }
    }
    
    // done, finding break type
    if (self->breakType > 0)
    {
      // TODO: parse first line
      ecstream_clear (self->line);
      
      (pd->len)++;
      self->state = 1;
      pd->pos = c;
      
      return TRUE;
    }
    
    //ecstream_appendc (self->line, *c);
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state1 (EcMultipartParser self, EcParserData* pd)
{
  int a = 0;
  const char* c;
  for (c = pd->pos; (pd->len > 0); c++)
  {
    (pd->len)--;
    
    // check break char
    if (((self->breakType == 2) && (*c == '\n')) || ((self->breakType == 1) && (*c == '\r')))
    {      
      ecstream_append_buf (self->line, pd->pos, a);

      pd->pos = c + 1;
      return ecmultipartparser_line (self, pd);
    }
    
    // check break char
    if ((self->breakType == 3) && (*c == '\r'))
    {
      ecstream_append_buf (self->line, pd->pos, a);

      self->state = 2;
      pd->pos = c + 1;

      return TRUE;
    }
    
    if (isNotAssigned (self->content) && ((*c == '\r')||(*c == '\n')))
    {
      eclogger_fmt (LL_WARN, "ENTC", "mime", "wrong line break type %i with [%i]", self->breakType, *c);      
      return FALSE;
    }
    
    a++;
  }
  
  ecstream_append_buf (self->line, pd->pos, a);
    
  if (self->content)
  {
    ecmultipartparser_isBoundary (self, ecstream_get(self->line), ecstream_size (self->line), FALSE);
  }
    
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state2 (EcMultipartParser self, EcParserData* pd)
{
  const char* c;
  for (c = pd->pos; (pd->len > 0); c++)
  {
    if ((self->breakType == 3) && (*c == '\n'))
    {
      (pd->len)--;

      self->state = 1;
      pd->pos = c + 1;
      
      return ecmultipartparser_line (self, pd);
    }
    
    if (self->content)
    {
      ecstream_append_c (self->line, '\r');
      self->state = 1;
      
      return TRUE;
    }
    else
    {
      eclogger_fmt (LL_WARN, "ENTC", "mime", "wrong sequence of breaks #3, break type %i with [%s]", self->breakType, *c); 
      return FALSE;
    }
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state3 (EcMultipartParser self, EcParserData* pd)
{
  const char* c;
  for (c = pd->pos; pd->len > 0; c++)
  {
    (pd->len)--;
    
    if (((self->breakType == 2) && (*c == '\n')) || ((self->breakType == 1) && (*c == '\r')))
    {
      // line break !!
      self->state = 1;
      return TRUE;      
    }
    
    if ((self->breakType == 3) && (*c == '\r'))
    {
      self->state = 4;
      return TRUE;
    }
    
    ecdevstream_appendc (self->devsteam, *c);
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state4 (EcMultipartParser self, EcParserData* pd)
{
  const char* c;
  for (c = pd->pos; pd->len > 0; c++)
  {
    (pd->len)--;
    
    if ((self->breakType == 3) && (*c == '\n'))
    {
      // line break !!
      self->state = 1;
      return TRUE;
    }
    
    ecdevstream_appendc (self->devsteam, '\r');
    ecdevstream_appendc (self->devsteam, *c);

    // continue
    self->state = 3;
    return TRUE;
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_checkStates (EcMultipartParser self, EcParserData* pd)
{
  switch (self->state)
  {
    case 0: return ecmultipartparser_state0 (self, pd);
    case 1: return ecmultipartparser_state1 (self, pd);
    case 2: return ecmultipartparser_state2 (self, pd);
    case 3: return ecmultipartparser_state3 (self, pd);
    case 4: return ecmultipartparser_state4 (self, pd);
  }
  
  return FALSE;
}

//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_processBuffer (EcMultipartParser self, const char* buffer, unsigned long size)
{
  EcParserData pd;
  
  pd.pos = buffer;
  pd.len = size;  
  pd.name = NULL;
  
  //ecfh_writeConst (self->fhDebug, buffer, size);
  
  //printf("\n======================================\n%s\n======================================[%u]\n", buffer, size);
  
  while (ecmultipartparser_checkStates (self, &pd));  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_process (EcMultipartParser self, ulong_t size)
{
  //self->fhDebug = ecfh_open ("debug.dat", O_CREAT | O_RDWR | O_TRUNC);
  
  if (size > 0)
  {
    EcStateParser sp;
    
    sp.state = 0;
    sp.read = 0;
    sp.size = size;

    while (sp.read < sp.size)
    {
      ulong_t res = 0;
      ulong_t diff = sp.size - sp.read;
      ulong_t max = diff < C_MAX_BUFSIZE ? diff : C_MAX_BUFSIZE;
      
      char* data = self->cb (self->ptr, (char*)self->buffer->buffer, max, &res);    
      
      if (res > 0)
      {
        ecmultipartparser_processBuffer (self, data, res);
        sp.read += res;
      }
      else
      {
        break;
      }
    }
  
  }
  else
  {
    while (TRUE)
    {
      ulong_t res;
      
      self->cb (self->ptr, (char*)self->buffer->buffer, self->buffer->size, &res);    

      if (res > 0)
      {
        ecmultipartparser_processBuffer (self, (char*)self->buffer->buffer, res);
      }
      else
      {
        break;
      }
    }
  }
  
  //ecfh_close (&(self->fhDebug));
  
  //eclogger_fmt (LL_DEBUG, "ENTC", "mime", "done parsing"); 
  
  return ENTC_RESCODE_OK;
}

//---------------------------------------------------------------------------------------

EcString echttpheader_parseLine (const EcString line, const EcString key)
{
  EcListCursor cursor;
  EcList tokens;
  EcString ret = NULL;
  int run = TRUE;
  
  tokens = ectokenizer_parse (line, ';');
  
  eclist_cursor_init (tokens, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    EcString token = ecstr_trim(eclist_data(cursor.node));
    EcString left = NULL;
    EcString right = NULL;
    
    if (ecstr_split (token, &left, &right, '='))
    {
      if (ecstr_equal(left, key))
      {
        ret = ecstr_trimc (right, '"');
        
        run = FALSE;
      }
    }
    
    ecstr_delete(&token);
    ecstr_delete(&left);
    ecstr_delete(&right);
  }
  
  eclist_destroy (&tokens);
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

struct EcMultipart_s {
  
  EcString boundary;
  
  EcString header;
  
  EcUdc sections;
  
  // state machine
  
  int state;
  
  EcBuffer buffer;
  
  ulong_t bufpos;
  
  EcUdc item;
  
  void* cursor;
  
  EcFileHandle fh;
  
};

//------------------------------------------------------------------------------------------------------

EcMultipart ecmultipart_create (const EcString boundary, const EcString header)
{
  EcMultipart self = ENTC_NEW (struct EcMultipart_s);
  
  if (isAssigned(boundary))
  {
    self->boundary = ecstr_copy(boundary);
  }
  else
  {
    EcBuffer buf = ecbuf_create_uuid ();
    
    self->boundary = ecbuf_str(&buf);
  }
  
  self->header = ecstr_copy (header);
  
  self->sections = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, NULL);
  
  // for the state machine
  self->state = 0;
  self->buffer = NULL;
  self->bufpos = 0;
  self->item = NULL;
  self->cursor = NULL;
  self->fh = NULL;
  
  return self;
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_destroy (EcMultipart* pself)
{
  EcMultipart self = *pself;

  ecstr_delete(&(self->boundary));
  ecstr_delete(&(self->header));
  
  ecudc_destroy(EC_ALLOC, &(self->sections));
  
  ENTC_DEL(pself, struct EcMultipart_s);
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_addText (EcMultipart self, const EcString text, const EcString mimeType)
{
  if (mimeType)
  {
    EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
    
    ecudc_add_asString (EC_ALLOC, h, "text", text);
    ecudc_add_asString (EC_ALLOC, h, "mime", mimeType);
    
    ecudc_add (self->sections, &h);
  }
  else
  {
    EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_STRING, NULL);
    
    ecudc_setS (h, text);
    
    ecudc_add (self->sections, &h);
  }
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_addFile (EcMultipart self, const EcString path, const EcString file, int fileId)
{
  EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_FILEINFO, NULL);

  EcFileInfo fi = ecudc_asFileInfo(h);
  
  fi->name = ecstr_copy (file);
  fi->path = ecfs_mergeToPath (path, file);
  fi->inode = fileId;
  
  ecudc_add (self->sections, &h);
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_addPath (EcMultipart self, const EcString path, const EcString name, int fileId)
{
  EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_FILEINFO, NULL);
  
  EcFileInfo fi = ecudc_asFileInfo(h);
  
  fi->name = ecstr_copy (name);
  fi->path = ecstr_copy (path);
  fi->inode = fileId;
  
  ecudc_add (self->sections, &h);
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_addContentDisposition_B_o (EcMultipart self, const EcString name, EcBuffer* pbuf)
{
  EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (EC_ALLOC, h, "name", name);
  ecudc_add_asB_o (EC_ALLOC, h, "data", pbuf);
  
  ecudc_add (self->sections, &h);
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_addContentDisposition_S (EcMultipart self, const EcString name, const EcString content)
{
  EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (EC_ALLOC, h, "name", name);
  ecudc_add_asString (EC_ALLOC, h, "data", content);
  
  ecudc_add (self->sections, &h);
}

//------------------------------------------------------------------------------------------------------

void ecmultipart_addContentDisposition_S_o (EcMultipart self, const EcString name, EcString* pcontent)
{
  EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, NULL);
  
  ecudc_add_asString (EC_ALLOC, h, "name", name);
  ecudc_add_asS_o (EC_ALLOC, h, "data", pcontent);
  
  ecudc_add (self->sections, &h);
}

//------------------------------------------------------------------------------------------------------

uint_t ecmultipart_nextState (EcMultipart self, EcBuffer buf, int newState)
{
  self->state = newState;
  return ecmultipart_next (self, buf);
}

//------------------------------------------------------------------------------------------------------

uint_t ecmultipart_handleBuffer (EcMultipart self, EcBuffer buf, int newState)
{
  ulong_t min = ENTC_MIN(buf->size, self->buffer->size - self->bufpos);
  
  //eclogger_fmt (LL_TRACE, "ENTC", "mime", "buffer %u", min);

  if (min == 0)
  {
    ecbuf_destroy (&(self->buffer));
    
    return ecmultipart_nextState (self, buf, newState);
  }
  else
  {
    memcpy (buf->buffer, self->buffer->buffer + self->bufpos, min);
    self->bufpos += min;
    
    return min;
  }
}

//------------------------------------------------------------------------------------------------------

#define MULTIPART_STATE_BEGIN 0
#define MULTIPART_STATE_NEXTITEM 1
#define MULTIPART_STATE_END 2
#define MULTIPART_STATE_TEXT 3
#define MULTIPART_STATE_FILE 4
#define MULTIPART_STATE_TERM 5
#define MULTIPART_STATE_NODE 6
#define MULTIPART_STATE_FILE_EOF 7

//------------------------------------------------------------------------------------------------------

EcString ecmultipart_startGetContentType (EcMultipart self)
{
  EcStream stream = ecstream_create ();
  EcBuffer h;
  
  ecstream_append_str (stream, "Content-Type: multipart/related; ");
  ecstream_append_str (stream, "boundary=\"");
  ecstream_append_str (stream, self->boundary);
  ecstream_append_str (stream, "\"");

  self->state = MULTIPART_STATE_NEXTITEM;
  
  h = ecstream_tobuf (&stream);
  return ecbuf_str(&h);
}

//------------------------------------------------------------------------------------------------------

uint_t ecmultipart_next (EcMultipart self, EcBuffer buf)
{
  switch (self->state)
  {
    case MULTIPART_STATE_BEGIN:
    {
      if (self->buffer == NULL)
      {
        EcStream stream = ecstream_create ();

        // start with header
        if (self->header)
        {
          ecstream_append_str (stream, self->header);
        }
        
        ecstream_append_str (stream, "MIME-Version: 1.0\r\n");
        ecstream_append_str (stream, "Content-Type: multipart/related; ");
        ecstream_append_str (stream, "boundary=\"");
        ecstream_append_str (stream, self->boundary);
        ecstream_append_str (stream, "\"");
        
        ecstream_append_str (stream, "\r\n\r\n");

        self->buffer = ecstream_tobuf (&stream);
        self->bufpos = 0;
      }
      
      return ecmultipart_handleBuffer (self, buf, MULTIPART_STATE_NEXTITEM);
    }
    case MULTIPART_STATE_NEXTITEM:
    {
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "nextitem");

      self->item = ecudc_next (self->sections, &(self->cursor));
      if (isAssigned(self->item))
      {
        switch (ecudc_type(self->item))
        {
          case ENTC_UDC_STRING: return ecmultipart_nextState (self, buf, MULTIPART_STATE_TEXT);
          case ENTC_UDC_FILEINFO: return ecmultipart_nextState (self, buf, MULTIPART_STATE_FILE);
          case ENTC_UDC_NODE: return ecmultipart_nextState (self, buf, MULTIPART_STATE_NODE);
          default: return ecmultipart_next (self, buf); // skip this
        }
      }
      else
      {
        // no items left
        return ecmultipart_nextState (self, buf, MULTIPART_STATE_END);
      }
    }
    case MULTIPART_STATE_END: // finalize
    {
      if (self->buffer == NULL)
      {
        EcStream stream = ecstream_create ();
      
        ecstream_append_str (stream, "--");
        ecstream_append_str (stream, self->boundary);
        ecstream_append_str (stream, "--");
        
        self->buffer = ecstream_tobuf (&stream);
        self->bufpos = 0;
      }
      
      return ecmultipart_handleBuffer (self, buf, MULTIPART_STATE_TERM);
    }
    case MULTIPART_STATE_TEXT: // text
    {
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "text");
      
      if (self->buffer == NULL)
      {
        EcStream stream = ecstream_create ();
        
        ecstream_append_str (stream, "--");
        ecstream_append_str (stream, self->boundary);
        ecstream_append_str (stream, "\r\nContent-Type: text/plain; charset=\"UTF-8\"\r\n\r\n");
        ecstream_append_str (stream, ecudc_asString(self->item));
        ecstream_append_str (stream, "\r\n");
        
        self->buffer = ecstream_tobuf (&stream);
        self->bufpos = 0;
      }
      
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "text done");

      return ecmultipart_handleBuffer (self, buf, MULTIPART_STATE_NEXTITEM);
    }
    case MULTIPART_STATE_NODE:
    {
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "node");

      if (self->buffer == NULL)
      {
        EcStream stream = ecstream_create ();
        
        const EcString name = ecudc_get_asString(self->item, "name", NULL);
        if (name)
        {
          EcUdc dataNode;

          ecstream_append_str (stream, "--");
          ecstream_append_str (stream, self->boundary);
          ecstream_append_str (stream, "\r\nContent-Disposition: name=\"");
          ecstream_append_str (stream, name);
          ecstream_append_str (stream, "\"\r\n\r\n");
          
          dataNode = ecudc_node(self->item, "data");
          if (dataNode)
          {
            if (ecudc_type(dataNode) == ENTC_UDC_BUFFER)
            {
              EcBuffer h = ecudc_asB (dataNode);
              
              ecstream_append_buf (stream, (const char*)h->buffer, h->size);
            }
            else if (ecudc_type(dataNode) == ENTC_UDC_STRING)
            {
              ecstream_append_str (stream, ecudc_asString(dataNode));
            }
          }
          
          ecstream_append_str (stream, "\r\n");
        }
        else
        {
          ecstream_append_str (stream, "--");
          ecstream_append_str (stream, self->boundary);
          ecstream_append_str (stream, "\r\nContent-Type: ");
          ecstream_append_str (stream, ecudc_get_asString(self->item, "mime", "text/plain; charset=\"UTF-8\""));
          ecstream_append_str (stream, "\r\n\r\n");
          ecstream_append_str (stream, ecudc_get_asString(self->item, "text", ""));
          ecstream_append_str (stream, "\r\n");
        }
        
        self->buffer = ecstream_tobuf (&stream);
        self->bufpos = 0;
      }

      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "node done");

      return ecmultipart_handleBuffer (self, buf, MULTIPART_STATE_NEXTITEM);
    }
    case MULTIPART_STATE_FILE: // file
    {
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "file");

      if (self->fh == NULL)
      {
        EcFileInfo fi = ecudc_asFileInfo(self->item);
        
        self->fh = ecfh_open (fi->path, O_RDONLY);
        if (isNotAssigned (self->fh))
        {
          eclogger_fmt (LL_WARN, "ENTC", "mime", "can't open file '%s'", fi->path);
          // in case of error just continue
          return ecmultipart_nextState (self, buf, MULTIPART_STATE_NEXTITEM);
        }
        {
          const EcString mimeType = ecmime_getFromFile (fi->path);
          
          EcStream stream = ecstream_create ();
          
          ecstream_append_str (stream, "--");
          ecstream_append_str (stream, self->boundary);
          ecstream_append_str (stream, "\r\n");
          ecstream_append_str (stream, "Content-Type: ");
          ecstream_append_str (stream, mimeType);
          ecstream_append_str (stream, "; name=\"");
          ecstream_append_str (stream, fi->name);
          ecstream_append_str (stream, "\"\r\n");
          ecstream_append_str (stream, "Content-Disposition: INLINE; filename=\"");
          ecstream_append_str (stream, fi->name);
          ecstream_append_str (stream, "\"\r\n");
          ecstream_append_str (stream, "Content-ID: <");
          ecstream_append_u64 (stream, fi->inode);
          ecstream_append_str (stream, ">");
          ecstream_append_str (stream, "\r\n");
          ecstream_append_str (stream, "Content-Transfer-Encoding: base64");
          ecstream_append_str (stream, "\r\n\r\n");
          
          self->buffer = ecstream_tobuf (&stream);
          self->bufpos = 0;
        }
      }
      else if (self->buffer == NULL)
      {
        ulong_t size = ecbuf_encode_base64_calculateSize (buf->size);
        
        EcBuffer h = ecbuf_create (size);
        
        uint_t readBytes = ecfh_readBuffer (self->fh, h);
        if (readBytes == 0) // EOF reached
        {
          ecbuf_destroy(&h);
          
          ecfh_close (&(self->fh));
          return ecmultipart_nextState (self, buf, MULTIPART_STATE_FILE_EOF);
        }
        else
        {
          ulong_t res = ecbuf_encode_base64_d (h, buf);
          
          ecbuf_destroy(&h);

          return res;
        }
      }

      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "file done");

      return ecmultipart_handleBuffer (self, buf, MULTIPART_STATE_FILE);
    }
    case MULTIPART_STATE_FILE_EOF:
    {
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "file eof");

      if (self->buffer == NULL)
      {
        EcStream stream = ecstream_create ();

        ecstream_append_str (stream, "\r\n");
        
        self->buffer = ecstream_tobuf (&stream);
        self->bufpos = 0;
      }
      
      //eclogger_fmt (LL_TRACE, "ENTC", "mime", "file eof done");

      return ecmultipart_handleBuffer (self, buf, MULTIPART_STATE_NEXTITEM);
    }
    case MULTIPART_STATE_TERM: // terminate
    {
      return 0;
    }
  }
  
  return 0;
}

//------------------------------------------------------------------------------------------------------

