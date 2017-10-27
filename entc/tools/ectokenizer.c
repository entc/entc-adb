
#include "ectokenizer.h"

// entc include
#include "system/macros.h"

//-----------------------------------------------------------------------------

static int __STDCALL ectokenizer_token_onDestroy (void* ptr)
{
  ENTC_FREE(ptr);
  
  return 0;
}

//-----------------------------------------------------------------------------

EcList ectokenizer_parse (const EcString source, char delimeter)
{
  EcList tokens;
  
  const char* posR = source;
  const char* posL = source;
  
  if (ecstr_empty (source))
  {
    return NULL;
  }

  tokens = eclist_create (ectokenizer_token_onDestroy);
  
  while (*posR)
  {
    if (*posR == delimeter)
    {
      // calculate the length of the last segment
      uint_t diff = posR - posL;
      if (diff > 0)
      {
        // allocate memory for the segment
        char* buffer = (char*)ENTC_MALLOC(diff + 2);
        memcpy(buffer, posL, diff);
        // set termination
        buffer[diff] = 0;
        // add to list
        eclist_push_back (tokens, buffer);
      }
      posL = posR + 1;
    }
    posR++;
  }
  
  {
    // calculate the length of the last segment
    uint_t diff = posR - posL;
    if (diff > 0)
    {
      // allocate memory for the segment
      char* buffer = (char*)ENTC_MALLOC(diff + 1);
      memcpy(buffer, posL, diff);
      // set termination
      buffer[diff] = 0;
      
      // add to list
      eclist_push_back (tokens, buffer);
    }
  }
  
  return tokens;
}

//-----------------------------------------------------------------------------
