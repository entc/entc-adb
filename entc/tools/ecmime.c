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

//-----------------------------------------------------------------------------------------------------------

#define C_MAX_BUFSIZE 10000

struct EcMultipartParser_s
{
  
  EcString boundary;
  
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
  
  EcDevStream devsteam;
  
  EcFileHandle fh; 
  
  EcString fn;
  
  EcMapChar params;
  
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

EcMultipartParser ecmultipartparser_create (const EcString boundary, const EcString path, http_content_callback cb, void* ptr, EcHttpContent hc)
{
  EcMultipartParser self = ENTC_NEW (struct EcMultipartParser_s);
  
  self->boundary = ecstr_cat2 ("--", boundary);
  
  self->cb =cb;
  self->ptr = ptr;
  self->hc = hc;
  
  self->path = ecstr_copy(path);
  
  self->buffer = ecbuf_create (C_MAX_BUFSIZE);
  self->line = ecstream_new ();
  self->params = ecmapchar_create (EC_ALLOC); 
  
  self->state = 0;
  self->breakType = 0;

  self->content = NULL;
  self->devsteam = NULL;
  self->fn = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_destroy (EcMultipartParser* pself)
{
  EcMultipartParser self = *pself;
  
  ecstr_delete(&(self->path));
  
  ecstr_delete (&(self->boundary));
  ecbuf_destroy (&(self->buffer));
  ecstream_delete (&(self->line));
  
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
    
    eclogger_fmt (LL_TRACE, "ENTC", "mime", "add param '%s':'%s'", key, val);
  }
  
  ecstr_delete (&key);
  ecstr_delete (&val);
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_line (EcMultipartParser self, EcParserData* pd)
{  
  // get last line
  const EcString line = ecstream_buffer (self->line);

  if (line [0] == 0)
  {
    //eclogger_fmt (LL_TRACE, "ENTC", "mime", "switch to content"); 

    self->content = ecstream_new ();
    
    /*
    if (self->state2 == 2)
    {
      self->state = 3;
      self->state2 = 3;
      
      {
        EcBuffer buf = ecbuf_create_uuid ();
        
        ecstr_replaceTO(&(self->fn), ecfs_mergeToPath(self->path, ecbuf_const_str(buf)));
        
        ecbuf_destroy (&buf);
      }
      
      self->fh = ecfh_open (self->fn, O_CREAT | O_RDWR);
      self->devsteam = ecdevstream_new (10000, ecmultipartparser_write, self);

      eclogger_fmt (LL_TRACE, "ENTC", "mime", "start content '%s'", self->fn); 
    }
     */
  }
  else
  {
    //eclogger_fmt (LL_TRACE, "ENTC", "mime", "line '%s'", line);
    
    // content state
    if (self->content)
    {
      // reached boundary
      if (ecstr_leading (line, self->boundary))
      {
        // create a new http content
        if (isAssigned (self->hc))
        {
          // transform stream to buffer
          EcBuffer buf = ecstream_trans (&(self->content));
          
          // create a new content part
          EcHttpContent hc = echttp_content_create2 (&buf, &(self->params));
        
          // assign the new content as next content
          self->hc = echttp_content_add (self->hc, &hc);
          
          // recreate an empty map
          self->params = ecmapchar_create (EC_ALLOC); 
        }
        else
        {
          ecstream_delete (&(self->content));
        }
      }
      else
      {
        // add line breaks
        if (ecstream_size (self->content) > 0)
        {
          switch (self->breakType)
          {
            case 1: ecstream_appendc (self->content, '\r'); break;
            case 2: ecstream_appendc (self->content, '\n'); break;
            case 3:
            {
              ecstream_appendc (self->content, '\r'); 
              ecstream_appendc (self->content, '\n'); 
            }
            break;
          }
        }
        
        // add to content
        ecstream_appendd (self->content, line, ecstream_size (self->line));
      }
    }
    else 
    {
      // add to parameters
      echttpheader_parseParam (self->params, line);
    }
    
    /*
    // check type
    if (ecstr_leading (line, "Content-Disposition: form-data;"))
    {
      if (self->state2 == 0)
      {
        self->state2 = 1;
      }
      
      ecstr_replace(&(pd->name), echttpheader_parseLine (line, "name"));      
    }
    else if (ecstr_equal (line, "Content-Type: application/octet-stream"))
    {
      self->state2 = 2;
    }
    else if (self->state2 == 1) // form data
    {
      if (ecstr_equal (line + 2, self->boundary))
      {
        self->state2 = 0;
      }
      else
      {
        eclogger_fmt (LL_TRACE, "ENTC", "mime", "got data '%s':'%s'", pd->name, line);          
      }
    }
    else if (self->devsteam)
    {
      if (ecstr_ending(line, self->boundary))
      {
        // done
        eclogger_fmt (LL_TRACE, "ENTC", "mime", "file wrote");          
        ecdevstream_delete(&(self->devsteam));
      }
    }
    else
    {
      eclogger_fmt (LL_WARN, "ENTC", "mime", "unknown linebreak '%s'", line);                
    }
     */
  }
  
  ecstream_clear (self->line);

  return TRUE;
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state0 (EcMultipartParser self, EcParserData* pd)
{
  // initial line
  // find what kind of line breaks we have and parse
  const char* c;
  for (c = pd->pos; c && (pd->len > 0); c++)
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
      
      self->state = 1;
      pd->pos = c;
      
      return TRUE;
    }
    
    ecstream_appendc (self->line, *c);
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state1 (EcMultipartParser self, EcParserData* pd)
{
  const char* c;
  for (c = pd->pos; pd->len > 0; c++)
  {
    (pd->len)--;
    
    // check break char
    if (((self->breakType == 2) && (*c == '\n')) || ((self->breakType == 1) && (*c == '\r')))
    {      
      ecstream_appendd (self->line, pd->pos, c - pd->pos);

      pd->pos = c + 1;
      return ecmultipartparser_line (self, pd);
    }
    
    // check break char
    if ((self->breakType == 3) && (*c == '\r'))
    {
      ecstream_appendd (self->line, pd->pos, c - pd->pos);

      self->state = 2;
      pd->pos = c + 1;

      return TRUE;
    }
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state2 (EcMultipartParser self, EcParserData* pd)
{
  const char* c;
  for (c = pd->pos; pd->len > 0; c++)
  {
    (pd->len)--;
    
    if ((self->breakType == 3) && (*c == '\n'))
    {
      self->state = 1;
      pd->pos = c + 1;
      
      return ecmultipartparser_line (self, pd);
    }
    
    ecstream_appendc (self->line, '\r'); 
    
    self->state = 1;
    return TRUE;
    
    if (self->devsteam)
    {
      // continueself->state = 3;
      
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
  pd.len = size + 1;
  pd.name = NULL;
  
 // eclogger_fmt (LL_TRACE, "ENTC", "mime", "buffer '%s'", buffer); 

  
  while (ecmultipartparser_checkStates (self, &pd));
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_process (EcMultipartParser self, ulong_t size)
{
  EcStateParser sp;
  
  sp.state = 0;
  sp.read = 0;
  sp.size = size;
  
  while (sp.read < sp.size)
  {
    int res = 0;

    ulong_t diff = sp.size - sp.read;
    ulong_t max = diff < C_MAX_BUFSIZE ? diff : C_MAX_BUFSIZE;
    
    sp.data = self->cb (self->ptr, (char*)self->buffer->buffer, max, &res);    
    if (res > 0)
    {
      ecmultipartparser_processBuffer (self, sp.data, res);
      sp.read += res;
    }

    //eclogger_fmt (LL_TRACE, "ENTC", "mime", "diff %i", diff); 
  }
  
  eclogger_fmt (LL_DEBUG, "ENTC", "mime", "done parsing"); 
  
  return ENTC_RESCODE_OK;
}

//---------------------------------------------------------------------------------------

EcString echttpheader_parseLine (const EcString line, const EcString key)
{
  EcListNode node;
  EcList tokens = eclist_create ();
  EcString ret = NULL;
  int run = TRUE;
  
  ecstr_tokenizer(tokens, line, ';');
  
  for (node = eclist_first(tokens); run && node != eclist_end(tokens); node = eclist_next(node))
  {
    EcString token = ecstr_trim(ecstr_tokenizer_get (tokens, node));
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
  
  ecstr_tokenizer_clear (tokens);
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------




