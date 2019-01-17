
#include "eclparser.h"

// entc include
#include "types/ecstream.h"

//-----------------------------------------------------------------------------

// state
#define SLP_STATE_TEXT     0
#define SLP_STATE_BR_R     1
#define SLP_STATE_BR_N     2

// mode
#define SLP_MODE_BR_NONE   0
#define SLP_MODE_BR_R      1
#define SLP_MODE_BR_N      2
#define SLP_MODE_BR_RN     3

//-----------------------------------------------------------------------------

struct EcLineParser_s
{
  fct_eclineparser_onLine onLine;
  
  void* ptr;
  
  int state;   // for the state machine
  
  int mode;
  
  EcStream stream;
  
};

//-----------------------------------------------------------------------------

EcLineParser eclineparser_create (fct_eclineparser_onLine onLine, void* ptr)
{
  EcLineParser self = ENTC_NEW(struct EcLineParser_s);
  
  self->onLine = onLine;
  self->ptr = ptr;
  
  self->state = SLP_STATE_TEXT;
  self->mode = SLP_MODE_BR_NONE;
  
  self->stream = ecstream_create ();
  
  return self;
}

//-----------------------------------------------------------------------------

void eclineparser_destroy (EcLineParser* pself)
{
  EcLineParser self = *pself;

  ecstream_destroy (&(self->stream));
  
  ENTC_DEL(pself, struct EcLineParser_s);
}

//-----------------------------------------------------------------------------

void eclineparser_recordLineBreak (EcLineParser self)
{
  if (ecstream_size (self->stream) > 0)
  {
    if (self->onLine)
    {
      self->onLine (self->ptr, ecstream_get (self->stream));
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

void eclineparser_parse (EcLineParser self, const char* buffer, int size, int last)
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
        switch (self->mode)
        {
          case SLP_MODE_BR_R:
          {
            // record line break
            eclineparser_recordLineBreak (self);
            break;            
          }
          case SLP_MODE_BR_N:
          {
            // wrong line break -> ignore ??
            break;
          }
          case SLP_MODE_BR_RN:
          {
            self->state = SLP_STATE_BR_R;
            break;
          }
          default: 
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
                // set new mode
                self->mode = SLP_MODE_BR_R;
                self->state = SLP_STATE_TEXT;
                
                // record line break
                eclineparser_recordLineBreak (self);
                break;
              }
              case SLP_STATE_BR_N:
              {
                // not supported mode
                self->state = SLP_STATE_TEXT;
                
                // record line break
                eclineparser_recordLineBreak (self);
                break;
              }
            }
            break;
          }
        }
        break;
      }
      case '\n':
      {
        switch (self->mode)
        {
          case SLP_MODE_BR_R:
          {
            // wrong line break -> ignore ??
            break;            
          }
          case SLP_MODE_BR_N:
          {
            // record line break            
            eclineparser_recordLineBreak (self);
            break;
          }
          case SLP_MODE_BR_RN:
          {
            if (self->state == SLP_STATE_BR_R)
            {
              self->state = SLP_STATE_TEXT;
              eclineparser_recordLineBreak (self);
            }
            else
            {
              // wrong line break -> ignore ??  
            }
            break;
          }
          default:
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
                // set new mode
                self->mode = SLP_MODE_BR_RN;
                self->state = SLP_STATE_TEXT;

                // record line break
                eclineparser_recordLineBreak (self);
                break;
              }
              case SLP_STATE_BR_N:
              {
                // set new mode
                self->mode = SLP_MODE_BR_N;
                self->state = SLP_STATE_TEXT;

                // record line break
                eclineparser_recordLineBreak (self);
                break;
              }
            }
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
            // set new mode
            self->mode = SLP_MODE_BR_R;
            self->state = SLP_STATE_TEXT;

            // record line break
            eclineparser_recordLineBreak (self);
            break;
          }
          case SLP_STATE_BR_N:
          {
            // set new mode
            self->mode = SLP_MODE_BR_N;
            self->state = SLP_STATE_TEXT;

            // record line break
            eclineparser_recordLineBreak (self);
            break;
          }
        }
        
        // record char
        ecstream_append_c (self->stream, *c);
        
        break;
      }
    }
  }
  
  switch (self->state)
  {
    case SLP_STATE_BR_R:
    {
      self->state = SLP_STATE_TEXT;
      
      // record line break
      eclineparser_recordLineBreak (self);
      break;
    }
    case SLP_STATE_BR_N:
    {
      self->state = SLP_STATE_TEXT;
      
      // record line break
      eclineparser_recordLineBreak (self);
      break;
    }
    case SLP_STATE_TEXT:
    {
      if (last)
      {
        eclineparser_recordLineBreak (self);    
      }
     
      break;
    }
  }
}

//-----------------------------------------------------------------------------
