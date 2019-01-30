#include "entc_tokenizer.h"


//-----------------------------------------------------------------------------

static void __STDCALL entc_tokenizer_onDestroy (void* ptr)
{
  EntcString h = ptr; entc_str_del (&h);
}

//-----------------------------------------------------------------------------------------------------------

EntcList entc_tokenizer_buf (const char* buffer, number_t len, char delimeter)
{
  EntcList tokens = entc_list_new (entc_tokenizer_onDestroy);
  
  const char* posR = buffer;
  const char* posL = buffer;
  
  number_t posI = 0;
  
  if (buffer == NULL)
  {
    return tokens;
  }
  
  for (posI = 0; posI < len; posI++)
  {
    if (*posR == delimeter)
    {
      // calculate the length of the last segment
      EntcString h = entc_str_sub (posL, posR - posL);

      entc_list_push_back (tokens, h);
      
      posL = posR + 1;
    }
    
    posR++;
  }
  
  {
    EntcString h = entc_str_sub (posL, posR - posL);
    
    entc_list_push_back (tokens, h);
  }
  
  return tokens;  
}

//-----------------------------------------------------------------------------------------------------------

int entc_tokenizer_split (const EntcString source, char token, EntcString* p_left, EntcString* p_right)
{
  if (source == NULL)
  {
    return FALSE;
  }
  {
    const char * pos = strchr (source, token);
    if (pos != NULL)
    {
      EntcString h = entc_str_sub (source, pos - source);
      
      entc_str_replace_mv (p_left, &h);
      entc_str_replace_cp (p_right, pos + 1);
      
      return TRUE;
    }
    
    return FALSE;
  }
}

//-----------------------------------------------------------------------------------------------------------
