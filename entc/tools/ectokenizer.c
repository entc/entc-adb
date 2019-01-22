#include "ectokenizer.h"

//-----------------------------------------------------------------------------

static void __STDCALL ectokenizer_token_onDestroy (void* ptr)
{
  ENTC_FREE(ptr);
}

//-----------------------------------------------------------------------------

EntcList ectokenizer_parse (const EcString source, char delimeter)
{
  EntcList tokens = entc_list_new (ectokenizer_token_onDestroy);
  
  const char* posR = source;
  const char* posL = source;
  
  if (ecstr_empty (source))
  {
    return tokens;
  }
  
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
        entc_list_push_back (tokens, buffer);
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
      entc_list_push_back (tokens, buffer);
    }
  }
  
  return tokens;
}

//-----------------------------------------------------------------------------

EntcList ectokenizer_token (const EcString source, const EcString token)
{
  EntcList tokens = entc_list_new (ectokenizer_token_onDestroy);

  // use a state machine to parse the string with a string
  int state = 0;
  
  // the positions
  const char* pos;
  const char* tok = token;
  
  // starts with a potential token
  const char* tbe;
  
  // last token ends
  const char* tbl = source;
  
  // the loop
  for (pos = source; *pos; pos++)
  {
    switch (state)
    {
      case 0:
      {
        // check the first character
        if (*pos == *tok)
        {
          state = 1;
          tok++;
          
          tbe = pos;  // remember beginning of the token 

          if (*tok == '\0')  // token found, termination reached
          {
            // extract the part
            EcString h = ecstr_part (tbl, tbe - tbl);

            // add to tokens
            entc_list_push_back (tokens, h);
            
            tbl = pos + 1;
            state = 0;
            tok = token;            
          }          

        }

        break;
      }
      case 1:
      {
        if (*pos == *tok)  // match is still ok, continue
        {
          tok++;
          
          if (*tok == '\0')  // token found, termination reached
          {
            // extract the part
            EcString h = ecstr_part (tbl, tbe - tbl);

            // add to tokens
            entc_list_push_back (tokens, h);
            
            tbl = pos + 1;
            state = 0;
            tok = token;            
          }          
        }        
        else  // no match -> go back to the last match
        {
          state = 0;
          tok = token;
          
          // correct position
          pos = tbe;
        }
       
        break;
      }
    }    
  }
  
  // add last part as token

  // extract the part
  EcString h = ecstr_part (tbl, pos - tbl);
  
  // add to tokens
  entc_list_push_back (tokens, h);
  
  return tokens;
}

//-----------------------------------------------------------------------------
