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

EcMultipartParser ecmultipartparser_create (const EcString boundary, http_content_callback cb, void* ptr)
{
  EcMultipartParser self = ENTC_NEW (struct EcMultipartParser_s);
  
  self->boundary = ecstr_copy (boundary);
  self->cb =cb;
  self->ptr = ptr;
  
  self->buffer = ecbuf_create (C_MAX_BUFSIZE);
  self->line = ecstream_new ();
  
  self->state = 0;
  self->breakType = 0;
  
  self->state2 = 0;
  
  return self;
}

//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_destroy (EcMultipartParser* pself)
{
  EcMultipartParser self = *pself;
  
  ecstr_delete (&(self->boundary));
  ecbuf_destroy (&(self->buffer));
  ecstream_delete (&(self->line));
  
  ENTC_DEL (pself, struct EcMultipartParser_s);
}

/*
//-----------------------------------------------------------------------------------------------------------

void ecmultipartparser_states (EcMultipartParser self, EcStateParser* sp)
{
  switch (sp->state)
  {
    case 0:  // search for boundary
    {      
      sp->b2 = self->boundary;

      for (; sp->bpos < sp->bsiz; (sp->bpos)++, (sp->b2)++, (sp->b1)++)
      {
        if (*(sp->b1) == *(sp->b2)) // found first char of the boundary
        {
          sp->state = 1;
          return;
        }        
        
        ecstream_appendc (self->content, *(sp->b1));
      }
      
      // if we reach here no boundary was found
      
    }
    break;
    case 1:
    {
      for (; sp->bpos < sp->bsiz; (sp->bpos)++, (sp->b2)++, (sp->b1)++)
      {
        if (*(sp->b1) == 0)   // OK, boundary found
        {
          const EcString val = ecstream_buffer (self->content);
          
          eclogger_fmt (LL_DEBUG, "ENTC", "mime", "boundary value '%s'", val);
                    
          ecstream_clear(self->content);
          
          sp->state = 0;
          (sp->b2)++;
          return;
        }
        else if (*(sp->b1) != *(sp->b2))   // NOT a boundary
        {
          sp->state = 0;
          return;
        }
      }
    }
    break;
  }
}

 */
 
//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_line (EcMultipartParser self, EcParserData* pd, const char* pos)
{
  pd->pos = pos;
  
  // get last line
  const EcString line = ecstream_buffer (self->line);

  if (line [0] == 0)
  {
    if (self->state2 == 2)
    {
      eclogger_fmt (LL_TRACE, "ENTC", "mime", "start content"); 
      self->state = 3;
    }
  }
  else
  {
    //eclogger_fmt (LL_TRACE, "ENTC", "mime", "line '%s'", line); 
    
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
      if (ecstr_ending (line, self->boundary))
      {
        self->state2 = 0;
      }
      else
      {
        eclogger_fmt (LL_TRACE, "ENTC", "mime", "got data '%s':'%s'", pd->name, line);          
      }
    }
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
      return ecmultipartparser_line (self, pd, c + 1);
    }
    
    // check break char
    if ((self->breakType == 3) && (*c == '\r'))
    {
      self->state = 2;
      pd->pos = c + 1;

      return TRUE;
    }
    
    ecstream_appendc (self->line, *c);
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
      return ecmultipartparser_line (self, pd, c + 1);
    }
    
    eclogger_fmt (LL_WARN, "ENTC", "mime", "wrong sequence of breaks #3, break type %i with [%s]", self->breakType, *c); 
    return FALSE;
  }
  
  return FALSE;  
}

//-----------------------------------------------------------------------------------------------------------

int ecmultipartparser_state3 (EcMultipartParser self, EcParserData* pd)
{
  //const char* c;
  //for (c = pd->pos; pd->len > 0; c++)
  {

    //eclogger_fmt (LL_DEBUG, "ENTC", "mime", "write size %i", pd->len); 
  
  
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

int ecmultipartparser_process (EcMultipartParser self, const EcString path, ulong_t size)
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




