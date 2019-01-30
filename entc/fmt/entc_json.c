#include "entc_json.h"

// cape includes
#include "fmt/entc_parser_json.h"
#include "stc/entc_stream.h"

//-----------------------------------------------------------------------------------------------------------

static void __STDCALL entc_json_onItem (void* ptr, void* obj, int type, void* val, const char* key, int index)
{
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_NODE:
    case ENTC_JPARSER_OBJECT_LIST:
    {
      EntcUdc h = val;
      
      entc_udc_add_name (obj, &h, key);
      break;
    }
    case ENTC_JPARSER_OBJECT_TEXT:
    {
      EntcUdc h = entc_udc_new (ENTC_UDC_STRING, key);
      
      entc_udc_set_s_cp (h, val);
      
      entc_udc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_NUMBER:
    {
      EntcUdc h = entc_udc_new (ENTC_UDC_NUMBER, key);
      
      number_t* dat = val;
      entc_udc_set_n (h, *dat);
      
      entc_udc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_FLOAT:
    {
      EntcUdc h = entc_udc_new (ENTC_UDC_FLOAT, key);
      
      double* dat = val;
      entc_udc_set_f (h, *dat);
      
      entc_udc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_BOLEAN:
    {
      EntcUdc h = entc_udc_new (ENTC_UDC_BOOL, key);
      
      long dat = (long)val;
      entc_udc_set_b (h, dat);
      
      entc_udc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_NULL:
    {
      break;
    }      
  }
}

//-----------------------------------------------------------------------------------------------------------

static void* __STDCALL entc_json_onObjCreate (void* ptr, int type)
{
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_NODE:
    {
      return entc_udc_new (ENTC_UDC_NODE, NULL);
    }
    case ENTC_JPARSER_OBJECT_LIST:
    {
      return entc_udc_new (ENTC_UDC_LIST, NULL);
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------------------------------------

static void __STDCALL entc_json_onObjDestroy (void* ptr, void* obj)
{
  EntcUdc h = obj; entc_udc_del (&h);
}

//-----------------------------------------------------------------------------

EntcUdc entc_json_from_buf (const char* buffer, number_t size, const EntcString name)
{
  EntcUdc ret = NULL;
  int res;
  
  EntcErr err = entc_err_new ();
  
  // create a new parser for the json format
  EntcParserJson parser_json = entc_parser_json_new (NULL, entc_json_onItem, entc_json_onObjCreate, entc_json_onObjDestroy);
 
  // try to parse the current buffer
  res = entc_parser_json_process (parser_json, buffer, size, err);
  if (res)
  {
    //eclog_msg (LL_ERROR, "JSON", "reader", err->text);
    //eclog_msg (LL_WARN, "JSON", "reader", buffer);
  }
  else
  {
    ret = entc_parser_json_object (parser_json);
    
    if (ret)
    {
      // set name
      entc_udc_set_name (ret, name);
    }
    else
    {
      //eclog_msg (LL_WARN, "JSON", "reader", "returned NULL object as node");
    }
  }
  
  // clean up
  entc_parser_json_del (&parser_json);
  
  entc_err_del (&err);
  
  return ret;
}

//-----------------------------------------------------------------------------

EntcUdc entc_json_from_s (const EntcString source, const EntcString name)
{
  if (source)
  {
    return entc_json_from_buf (source, strlen(source), name);
  }
  else
  {
    return NULL;
  }
}

//-----------------------------------------------------------------------------------------------------------

void entc_json_escape (EntcStream stream, const EntcString source)
{
  if (source)
  {
    const unsigned char* c = (const unsigned char*)source;
    for (; *c; c++)
    {
      switch (*c)
      {
        case '"':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, '"');
          break;
        }
        case '\\':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, '\\');
          break;
        }
        /*
         *        case '/':
         *        {
         *          ecstream_append_c (stream, '\\');
         *          ecstream_append_c (stream, '/');
         *          break;
      }
      */
        case '\r':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, 'r');
          break;
        }
        case '\n':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, 'n');
          break;
        }
        case '\t':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, 't');
          break;
        }
        case '\b':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, 'b');
          break;
        }
        case '\f':
        {
          entc_stream_append_c (stream, '\\');
          entc_stream_append_c (stream, 'f');
          break;
        }
        default:
        {
          // implementation taken from https://gist.github.com/rechardchen/3321830
          // 
          if (0x20 <= *c && *c <= 0x7E) // )
          {
            //printf ("UTF8 [0] %c\n", *c);
            
            entc_stream_append_c (stream, *c);
          }
          else if ((*c & 0xE0) == 0xC0) 
            //  (0xC2 <= c[0] && c[0] <= 0xDF) && (0x80 <= c[1] && c[1] <= 0xBF))
          {
            char buffer[8];
            // convert UTF8 into 4 hex digits 
            wchar_t wc;
            
            wc = (c[0] & 0x1F) << 6;
            wc |= (c[1] & 0x3F);
            
            #if defined _WIN64 || defined _WIN32
            sprintf_s (buffer, 8, "\\u%.4x", wc);
            #else
            // TODO: there might be a better way to do it
            sprintf (buffer, "\\u%.4x", wc);
            #endif
            
            entc_stream_append_buf (stream, buffer, 6);
            
            c += 1;
          }
          else if ((*c & 0xF0) == 0xE0) 
            /* (c[0] == 0xE0 && (0xA0 <= c[1] && c[1] <= 0xBF) && (0x80 <= c[2] && c[2] <= 0xBF)) ||
             *            (// straight 3-byte
             *                    ((0xE1 <= c[0] && c[0] <= 0xEC) || c[0] == 0xEE || c[0] == 0xEF) &&
             *                    (0x80 <= c[1] && c[1] <= 0xBF) &&
             *                    (0x80 <= c[2] && c[2] <= 0xBF)
             *                ) ||
             *                (// excluding surrogates
             *                    c[0] == 0xED &&
             *                    (0x80 <= c[1] && c[1] <= 0x9F) &&
             *                    (0x80 <= c[2] && c[2] <= 0xBF))) 
             */
            {
              char buffer[8];
              wchar_t wc;
              
              wc = (c[0] & 0xF) << 12;
              wc |= (c[1] & 0x3F) << 6;
              wc |= (c[2] & 0x3F);
              
              c += 2;
              
              #if defined _WIN64 || defined _WIN32
              sprintf_s (buffer, 8, "\\u%.4x", wc);
              #else
              // TODO: there might be a better way to do it
              sprintf (buffer, "\\u%.4x", wc);
              #endif
              entc_stream_append_buf (stream, buffer, 6); 
            }
            else if ( (*c & 0xF8) == 0xF0 )
              /*( (// planes 1-3
               *                    c[0] == 0xF0 &&
               *                    (0x90 <= c[1] && c[1] <= 0xBF) &&
               *                    (0x80 <= c[2] && c[2] <= 0xBF) &&
               *                    (0x80 <= c[3] && c[3] <= 0xBF)
               *                ) ||
               *                (// planes 4-15
               *                    (0xF1 <= c[0] && c[0] <= 0xF3) &&
               *                    (0x80 <= c[1] && c[1] <= 0xBF) &&
               *                    (0x80 <= c[2] && c[2] <= 0xBF) &&
               *                    (0x80 <= c[3] && c[3] <= 0xBF)
               *                ) ||
               *                (// plane 16
               *                    c[0] == 0xF4 &&
               *                    (0x80 <= c[1] && c[1] <= 0x8F) &&
               *                    (0x80 <= c[2] && c[2] <= 0xBF) &&
               *                    (0x80 <= c[3] && c[3] <= 0xBF)
               *                )
               *          ) */
              {
                char buffer[8];
                wchar_t wc;
                
                wc = (c[0] & 0x7) << 18;
                wc |= (c[1] & 0x3F) << 12;
                wc |= (c[2] & 0x3F) << 6;
                wc |= (c[3] & 0x3F);
                
                c += 3;
                
                #if defined _WIN64 || defined _WIN32
                sprintf_s (buffer, 8, "\\u%.4x", wc);
                #else
                // TODO: there might be a better way to do it
                sprintf (buffer, "\\u%.4x", wc);
                #endif
                
                entc_stream_append_buf (stream, buffer, 6); 
              }
              else if ( (*c & 0xFC) == 0xF8 )
              {
                char buffer[8];
                wchar_t wc;
                
                wc = (c[0] & 0x3) << 24;
                wc |= (c[1] & 0x3F) << 18;
                wc |= (c[2] & 0x3F) << 12;
                wc |= (c[3] & 0x3F) << 6;
                wc |= (c[4] & 0x3F);
                
                c += 4;
                
                #if defined _WIN64 || defined _WIN32
                sprintf_s (buffer, 8, "\\u%.4x", wc);
                #else
                // TODO: there might be a better way to do it
                sprintf (buffer, "\\u%.4x", wc);
                #endif
                
                entc_stream_append_buf (stream, buffer, 6); 
              }
              else if ( (*c & 0xFE) == 0xFC )
              {
                char buffer[8];
                wchar_t wc;
                
                wc = (c[0] & 0x1) << 30;
                wc |= (c[1] & 0x3F) << 24;
                wc |= (c[2] & 0x3F) << 18;
                wc |= (c[3] & 0x3F) << 12;
                wc |= (c[4] & 0x3F) << 6;
                wc |= (c[5] & 0x3F);
                
                c += 5;
                
                #if defined _WIN64 || defined _WIN32
                sprintf_s (buffer, 8, "\\u%.4x", wc);
                #else
                // TODO: there might be a better way to do it
                sprintf (buffer, "\\u%.4x", wc);
                #endif
                
                entc_stream_append_buf (stream, buffer, 6); 
              }
              else
              {
                // not supported character
              }
        }
      }
    }
  }
  else
  {
    entc_stream_append_str (stream, "");
  }
}

//-----------------------------------------------------------------------------------------------------------

void entc_json_fill (EntcStream stream, const EntcUdc node)
{
  switch (entc_udc_type (node))
  {
    case ENTC_UDC_LIST:
    {
      EntcUdcCursor* cursor = entc_udc_cursor_new (node, ENTC_DIRECTION_FORW);

      entc_stream_append_c (stream, '[');

      while (entc_udc_cursor_next (cursor))
      {
        if (cursor->position)
        {
          entc_stream_append_c (stream, ',');
        }
        
        entc_json_fill (stream, cursor->item);
      }

      entc_stream_append_c (stream, ']');

      entc_udc_cursor_del (&cursor);

      break;
    }
    case ENTC_UDC_NODE:
    {
      EntcUdcCursor* cursor = entc_udc_cursor_new (node, ENTC_DIRECTION_FORW);
      
      entc_stream_append_c (stream, '{');
      
      while (entc_udc_cursor_next (cursor))
      {
        if (cursor->position)
        {
          entc_stream_append_c (stream, ',');
        }
        
        entc_stream_append_c (stream, '"');
        
        entc_json_escape (stream, entc_udc_name (cursor->item));
        
        entc_stream_append_str (stream, "\":");

        entc_json_fill (stream, cursor->item);
      }
      
      entc_stream_append_c (stream, '}');
      
      entc_udc_cursor_del (&cursor);

      break;
    }
    case ENTC_UDC_STRING:
    {
      const EntcString h = entc_udc_s (node, NULL);

      entc_stream_append_c (stream, '"');

      entc_json_escape (stream, h);
      
      entc_stream_append_c (stream, '"');

      break;
    }
    case ENTC_UDC_BOOL:
    {
      entc_stream_append_str (stream, entc_udc_b (node, FALSE) ? "true" : "false");
      break;
    }
    case ENTC_UDC_NUMBER:
    {
      entc_stream_append_n (stream, entc_udc_n (node, 0));
      break;
    }
    case ENTC_UDC_FLOAT:
    {
      entc_stream_append_f (stream, entc_udc_f (node, 0));
      break;
    }
  }
}

//-----------------------------------------------------------------------------

EntcString entc_json_to_s (const EntcUdc source)
{
  EntcStream stream = entc_stream_new ();
  
  entc_json_fill (stream, source);
  
  return entc_stream_to_str (&stream);
}

//-----------------------------------------------------------------------------
