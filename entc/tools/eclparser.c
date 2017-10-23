
#include "eclparser.h"

// entc include
#include "system/macros.h"
#include "types/ecstream.h"

//-----------------------------------------------------------------------------

#define SLP_STATE_TEXT     0
#define SLP_STATE_BR_R     1
#define SLP_STATE_BR_N     2

//-----------------------------------------------------------------------------

struct EcLineParser_s
{
  fct_stdlineparser_onLine onLine;
  
  void* ptr;
  
  int state;   // for the state machine
  
  EcStream stream;
  
};

//-----------------------------------------------------------------------------

EcLineParser eclineparser_create (fct_stdlineparser_onLine onLine, void* ptr)
{
  EcLineParser self = ENTC_NEW(struct EcLineParser_s);
  
  self->onLine = onLine;
  self->ptr = ptr;
  
  self->state = SLP_STATE_TEXT;
  
  self->stream = ecstream_new();
  
  return self;
}

//-----------------------------------------------------------------------------

void eclineparser_destroy (EcLineParser* pself)
{
  EcLineParser self = *pself;

  ecstream_delete(&(self->stream));
  
  ENTC_DEL(pself, struct EcLineParser_s);
}

//-----------------------------------------------------------------------------

void eclineparser_recordLineBreak (EcLineParser self)
{
  if (ecstream_size (self->stream) > 0)
  {
    if (self->onLine)
    {
      self->onLine (self->ptr, ecstream_buffer (self->stream));
    }
    
    ecstream_clear (self->stream);
  }
  else
  {
    if (self->onLine)
    {
      self->onLine (self->ptr, NULL);
    }
  }
}

//-----------------------------------------------------------------------------

void eclineparser_parse (EcLineParser self, const char* buffer, int size)
{
  int i;
  const char* c = buffer;
  
  for (i = 0; i < size; i++, c++)
  {
    switch (*c)
    {
      case 0:
      {
        // string terminated
        return;
      }
      case '\r':
      {
        switch (self->state)
        {
          case SLP_STATE_TEXT:
          {
            self->state = SLP_STATE_BR_R;
            break;
          }
          case SLP_STATE_BR_R:
          {
            // record line break
            eclineparser_recordLineBreak (self);
            
            break;
          }
          case SLP_STATE_BR_N:
          {
            // record line break
            eclineparser_recordLineBreak (self);
            
            self->state = SLP_STATE_BR_R;
            break;
          }
        }
        break;
      }
      case '\n':
      {
        switch (self->state)
        {
          case SLP_STATE_TEXT:
          {
            self->state = SLP_STATE_BR_N;
            break;
          }
          case SLP_STATE_BR_R:
          {
            self->state = SLP_STATE_BR_N;
            break;
          }
          case SLP_STATE_BR_N:
          {
            // record line break
            eclineparser_recordLineBreak (self);
            
            break;
          }
        }
        break;
      }
      default:
      {
        switch (self->state)
        {
          case SLP_STATE_BR_R:
          {
            // record line break
            eclineparser_recordLineBreak (self);
            
            self->state = SLP_STATE_TEXT;
            break;
          }
          case SLP_STATE_BR_N:
          {
            // record line break
            eclineparser_recordLineBreak (self);
            
            self->state = SLP_STATE_TEXT;
            break;
          }
        }
        
        // record char
        ecstream_appendc(self->stream, *c);
        
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------

void eclineparser_done (EcLineParser self)
{
  eclineparser_recordLineBreak (self);
  
  switch (self->state)
  {
    case SLP_STATE_BR_R:
    {
      // record line break
      eclineparser_recordLineBreak (self);
      
      break;
    }
    case SLP_STATE_BR_N:
    {
      // record line break
      eclineparser_recordLineBreak (self);
      
      break;
    }
  }
}

//-----------------------------------------------------------------------------
