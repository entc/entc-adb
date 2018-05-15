#include "ecjson.h"

#include "types/ecstream.h"
#include "tools/eclog.h"
#include "tools/eccrypt.h"
#include "tools/ecjparser.h"

#include <string.h>

//-----------------------------------------------------------------------------------------------------------

#define STATE_NONE 0
#define STATE_OBJECT 1
#define STATE_ARRAY 2

typedef struct {
  
  const unsigned char* pos;
  
} JsonParser;

//-------------------------------------------------------------------------------------------

#define STR_STATE_NONE        0
#define STR_STATE_BACKSLASH   1

// implemented as state machine
EcString json_parse_string (JsonParser* parser)
{
  int state = STR_STATE_NONE;
  const unsigned char* c;  
  
  for (c = parser->pos; *c; c++)
  {
    switch (state)
    {
      case STR_STATE_NONE: switch (*c)
      {
        case '\"':
        {
          // done
          EcString res = ecstr_part ((EcString)parser->pos, c - parser->pos);
          parser->pos = c;
          return res;
        }
          break;
        case '\\':
        {
          // enter escape state
          state = STR_STATE_BACKSLASH;
        }
          break;
      }
        break;
      case STR_STATE_BACKSLASH: switch (*c)
      {
          // ignore those characters
        case '\"': case '\\': case '/' : case 'b' : case 'f' : case 'r' : case 'n'  : case 't' :
        {
          // return none state
          state = STR_STATE_NONE;
        }
          break;
        case 'u':
        {
          int i = 0;
          c++;
          for(; (i < 4) && *c; i++, c++)
          {
            // If it isn't a hex character we have an error 
            if(!((*c >= 48 && *c <= 57) || // 0-9 
                 (*c >= 65 && *c <= 70) || // A-F 
                 (*c >= 97 && *c <= 102))) // a-f 
            { 
              return NULL;
            }
          }
          c--;
          // return none state
          state = STR_STATE_NONE;
        }
          break;
        default:
        {
          return NULL;
        }
      }
        break;
    }
  }
  return NULL;
}

//-------------------------------------------------------------------------------------------

EcUdc json_parse_type (JsonParser* parser, const EcString key, int len, int offset)
{
  EcUdc udc = NULL;
  
  EcString h1 = ecstr_part ((const char*)parser->pos + offset, len);
  EcString h2 = ecstr_trim (h1);

  if (ecstr_equal (h2, "null"))
  {
    udc = ecudc_create(EC_ALLOC, ENTC_UDC_NONE, key);    
  }
  else if (ecstr_equal (h2, "true"))
  {
    udc = ecudc_create(EC_ALLOC, ENTC_UDC_BOOL, key);
    ecudc_setBool (udc, TRUE);
  }
  else if (ecstr_equal (h2, "false"))
  {
    udc = ecudc_create(EC_ALLOC, ENTC_UDC_BOOL, key);
    ecudc_setBool (udc, FALSE);
  }
  else if (ecstr_has (h2, '.'))
  {
    udc = ecudc_create(EC_ALLOC, ENTC_UDC_DOUBLE, key);
    ecudc_setDouble(udc, atof(h2));
  }
  else
  {
    udc = ecudc_create(EC_ALLOC, ENTC_UDC_NUMBER, key);
    ecudc_setNumber (udc, atoi(h2));
  }
  
  ecstr_delete(&h2);
  ecstr_delete(&h1);
  
  return udc;
}

//-------------------------------------------------------------------------------------------

#define ENT_STATE_NONE            0
#define ENT_STATE_ARRAY           1
#define ENT_STATE_ARRAY_EOE       2
#define ENT_STATE_OBJECT          3
#define ENT_STATE_OBJECT_KEY      4
#define ENT_STATE_OBJECT_KEY_EOE  5
#define ENT_STATE_OBJECT_EOE      6

//-------------------------------------------------------------------------------------------

void* json_cleanup_udc (EcUdc* pudc)
{
  if (isAssigned (*pudc))
  {
    ecudc_destroy(EC_ALLOC, pudc);
  }
  return NULL;
}

//-------------------------------------------------------------------------------------------

// implemented as state machine
EcUdc json_parse (JsonParser* parser, const char* name)
{
  int state = ENT_STATE_NONE;
  const unsigned char* c;  
  EcUdc udc = NULL;
  EcString key = ecstr_init();
  
  for (c = parser->pos; *c; c++)
  {
    switch (state)
    {
      case ENT_STATE_NONE: switch (*c)
      {
        case '[':
        {
          if (isAssigned (udc))
          {
            // error
            eclog_msg (LL_TRACE, "JSON", "reader", "array already assigned");
            
            ecstr_delete(&key);            
            return json_cleanup_udc (&udc);
          }
          
          udc = ecudc_create (EC_ALLOC, ENTC_UDC_LIST, name); 
          
          parser->pos = c;
          state = ENT_STATE_ARRAY;
        }
        break;
        case '{':
        {
          if (isAssigned (udc))
          {
            ecstr_delete(&key);            
            // error
            eclog_msg (LL_TRACE, "JSON", "reader", "array already assigned");
            return json_cleanup_udc (&udc);
          }
          
          udc = ecudc_create (EC_ALLOC, ENTC_UDC_NODE, name); 
          
          parser->pos = c;
          state = ENT_STATE_OBJECT_KEY;
        }
        break;
      }
      break;
      case ENT_STATE_ARRAY: switch (*c)
      {
        case ']':
        {
          /*
          EcUdc h = json_parse_type (parser, NULL, c - parser->pos - 1, 1);
          if (h)
          {
            ecudc_add (udc, &h);
          }
           
          ecstr_delete(&key);
           */
          // todo
          parser->pos = c;
          return udc;
        }
        break;
        case '[': case '{':
        {
          parser->pos = c;
          {
            EcUdc child = json_parse (parser, "");
            if (isNotAssigned (child))
            {
              ecstr_delete(&key);            
              return json_cleanup_udc (&udc);
            }
            ecudc_add (udc, &child);
          }
          c = parser->pos;
          state = ENT_STATE_ARRAY_EOE;
        }
        break;
        case '\"':
        {
          parser->pos = c + 1;
          {
            EcString value = json_parse_string (parser);
            if (isNotAssigned (value))
            {
              ecstr_delete(&key);            
              return json_cleanup_udc (&udc);
            }
            ecudc_add_asString(EC_ALLOC, udc, "", value);
            ecstr_delete(&value);
          }
          c = parser->pos;
          state = ENT_STATE_ARRAY_EOE;
        }
        break;
        case ',':
        {
          /*
          EcUdc h = json_parse_type (parser, NULL, c - parser->pos - 1, 1);
          if (h)
          {
            ecudc_add (udc, &h);
          }
           */
           
          parser->pos = c;
        }
        break;
        // ignore
        case '\t' : case '\r' : case '\n' : case ' ': 
        break;
      }
      break;
      case ENT_STATE_ARRAY_EOE: switch (*c)
      {
        case ']':
        {
          ecstr_delete(&key);            
          parser->pos = c;
          return udc;
        }
        break;
        case ',':
        {
          parser->pos = c;
          state = ENT_STATE_ARRAY;
        }
        break;
          // ignore
        case '\t' : case '\r' : case '\n' : case ' ': 
        break;
          // error
        default:
        {
          ecstr_delete(&key);            
          return json_cleanup_udc (&udc);
        }
      }
      break;
      case ENT_STATE_OBJECT_KEY: switch (*c)
      {
        case '\"':
        {
          parser->pos = c + 1;
          {
            ecstr_replaceTO (&key, json_parse_string (parser));
            if (ecstr_empty (key))
            {
              ecstr_delete(&key);            
              return json_cleanup_udc (&udc);
            }
          }
          c = parser->pos;
          state = ENT_STATE_OBJECT_KEY_EOE;
        }
        break;
        // ignore
        case '\t' : case '\r' : case '\n' : case ' ':
        break;
        case '}': // empty node
        {
          parser->pos = c;
          return udc;
        }
        break;
        // error
        default:
        {
          ecstr_delete(&key);            
          return json_cleanup_udc (&udc);          
        }
      }
      break;
      case ENT_STATE_OBJECT_KEY_EOE: switch (*c)
      {
        case ':': state = ENT_STATE_OBJECT;
          break;
          // ignore
        case '\t' : case '\r' : case '\n' : case ' ': 
          break;
          // error
        default:
        {
          ecstr_delete(&key);            
          return json_cleanup_udc (&udc);
        }
      }
      break;
      case ENT_STATE_OBJECT: switch (*c)
      {
        case '}':
        {
          EcUdc h = json_parse_type (parser, key, c - parser->pos - 2, 2);
          if (h)
          {
            ecudc_add (udc, &h);
          }
          
          ecstr_delete(&key);            
          // todo
          parser->pos = c;
          return udc;
        }
        break;
        case '[': case '{':
        {
          parser->pos = c;
          {
            EcUdc child = json_parse (parser, key);
            if (isNotAssigned (child))
            {
              ecstr_delete(&key);            
              return json_cleanup_udc (&udc);
            }
            ecudc_add (udc, &child);
          }
          c = parser->pos;
          state = ENT_STATE_OBJECT_EOE;
        }
        break;
        case '\"':
        {
          parser->pos = c + 1;
          {
            EcString value = json_parse_string (parser);
            if (isNotAssigned (value))
            {
              ecstr_delete(&key);            
              return json_cleanup_udc (&udc);
            }
            ecudc_add_asString(EC_ALLOC, udc, key, value);
            ecstr_delete(&value);
          }
          c = parser->pos;
          state = ENT_STATE_OBJECT_EOE;
        }
        break;
        case ',':
        {
          EcUdc h = json_parse_type (parser, key, c - parser->pos - 2, 2);
          if (h)
          {
            ecudc_add (udc, &h);
          }

          state = ENT_STATE_OBJECT_KEY;
          parser->pos = c;
        }
        break;
        // ignore
        case '\t' : case '\r' : case '\n' : case ' ': break;
      }
      break;
      case ENT_STATE_OBJECT_EOE: switch (*c)
      {
        case '}':
        {
          ecstr_delete(&key);            
          parser->pos = c;
          return udc;
        }
          break;
        case ',':
        {
          parser->pos = c;
          state = ENT_STATE_OBJECT_KEY;
        }
          break;
          // ignore
        case '\t' : case '\r' : case '\n' : case ' ': 
          break;
          // error
        default:
        {
          ecstr_delete(&key);            
          return json_cleanup_udc (&udc);
        }
      }
      break;
    }
  }
  ecstr_delete(&key);            
  return json_cleanup_udc (&udc);
}

//-----------------------------------------------------------------------------------------------------------

static void __STDCALL ecjson_read_onItem (void* ptr, void* obj, int type, void* val, const char* key, int index)
{
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_NODE:
    case ENTC_JPARSER_OBJECT_LIST:
    {
      EcUdc h = val;
      ecudc_setName (h, key);
      
      ecudc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_TEXT:
    {
      EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_STRING, key);
      
      ecudc_setS(h, val);
     
      ecudc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_NUMBER:
    {
      EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NUMBER, key);
      
      unsigned long* dat = val;
      ecudc_setNumber(h, *dat);
      
      ecudc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_FLOAT:
    {
      EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_DOUBLE, key);
      
      double* dat = val;
      ecudc_setDouble(h, *dat);
      
      ecudc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_BOLEAN:
    {
      EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_BOOL, key);
      
      long dat = (long)val;
      ecudc_setBool(h, dat);
      
      ecudc_add (obj, &h);
      break;
    }
    case ENTC_JPARSER_OBJECT_NULL:
    {
      EcUdc h = ecudc_create (EC_ALLOC, ENTC_UDC_NONE, key);
      
      ecudc_add (obj, &h);
      break;
    }      
  }
}

//-----------------------------------------------------------------------------------------------------------

static void* __STDCALL ecjson_read_onObjCreate (void* ptr, int type)
{
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_NODE:
    {
      return ecudc_create(EC_ALLOC, ENTC_UDC_NODE, NULL);
    }
    case ENTC_JPARSER_OBJECT_LIST:
    {
      return ecudc_create(EC_ALLOC, ENTC_UDC_LIST, NULL);
    }
  }
  
  return NULL;
}

//-----------------------------------------------------------------------------------------------------------

static void __STDCALL ecjson_read_onObjDestroy (void* ptr, void* obj)
{
  EcUdc h = obj;
  ecudc_destroy (EC_ALLOC, &h);
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecjson_read_buffer (const char* buffer, int64_t size, const EcString name)
{
  EcUdc ret = NULL;
  int res;
  
  EcErr err = ecerr_create();
  EcJsonParser jparser = ecjsonparser_create (ecjson_read_onItem, ecjson_read_onObjCreate, ecjson_read_onObjDestroy, NULL);
  
  res = ecjsonparser_parse (jparser, buffer, size, err);
  if (res)
  {
    eclog_msg (LL_ERROR, "JSON", "reader", err->text);
    eclog_msg (LL_WARN, "JSON", "reader", buffer);
  }
  else
  {
    ret = ecjsonparser_lastObject (jparser);
  }
  
  if (ret)
  {
    // set name
    ecudc_setName (ret, name);
  }
  
  // clean up
  ecjsonparser_destroy (&jparser);
  ecerr_destroy(&err);
  
  return ret;
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecjson_read_ecbuf (const EcBuffer buf, const EcString name)
{
  return ecjson_read_buffer ((char*)buf->buffer, buf->size, name);
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecjson_read_s (const EcString source, const EcString name)
{
  if (source)
  {
    return ecjson_read_buffer (source, ecstr_len(source), name);
  }
  else
  {
    return NULL;
  }
}

//-----------------------------------------------------------------------------------------------------------

void jsonwriter_escape (EcStream stream, const EcString source)
{
  if (ecstr_valid (source))
  {
    const unsigned char* c;
    for (c = source; *c; c++)
    {
      switch (*c)
      {
        case '"':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, '"');
          break;
        }
        /*
        case '\'':
        {
          ecstream_append_str (stream, "\\u0027");
          break;
        }
        */
        case '\\':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, '\\');
          break;
        }
        case '/':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, '/');
          break;
        }
        case '\r':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 'r');
          break;
        }
        case '\n':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 'n');
          break;
        }
        case '\t':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 't');
          break;
        }
        case '\b':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 'b');
          break;
        }
        case '\f':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 'f');
          break;
        }
        default:
        {
          if ((*c & 0x80) == 0) // 0x20 <= *c && *c <= 0x7E)
          {
            //printf ("UTF8 [0] %c\n", *c);
            
            ecstream_append_c (stream, *c);
          }
          else if ((*c & 0xE0) == 0xC0) //  (0xC2 <= c[0] && c[0] <= 0xDF) && (0x80 <= c[1] && c[1] <= 0xBF))
          {
            char buffer[7];
            // convert UTF8 into 4 hex digits 
            wchar_t wc;
            
            wc = (c[0] & 0x1F) << 6;
            wc |= (c[1] & 0x3F);

            sprintf (buffer, "\\u%.4x\n", wc);             

            ecstream_append_buf (stream, buffer, 6);
            
            c += 1;
          }
          else if ((*c & 0xF0) == 0xE0) // (c[0] == 0xE0 && (0xA0 <= c[1] && c[1] <= 0xBF) && (0x80 <= c[2] && c[2] <= 0xBF)) ||
            /*   
            (// straight 3-byte
                    ((0xE1 <= c[0] && c[0] <= 0xEC) || c[0] == 0xEE || c[0] == 0xEF) &&
                    (0x80 <= c[1] && c[1] <= 0xBF) &&
                    (0x80 <= c[2] && c[2] <= 0xBF)
                ) ||
                (// excluding surrogates
                    c[0] == 0xED &&
                    (0x80 <= c[1] && c[1] <= 0x9F) &&
                    (0x80 <= c[2] && c[2] <= 0xBF))) 
                    */
          {
            char buffer[7];
            wchar_t wc;
            
            wc = (c[0] & 0xF) << 12;
            wc |= (c[1] & 0x3F) << 6;
            wc |= (c[2] & 0x3F);
            
            c += 2;

            sprintf (buffer, "\\u%.4x\n", wc);             

            ecstream_append_buf (stream, buffer, 6); 
          }
          else if( (// planes 1-3
                    c[0] == 0xF0 &&
                    (0x90 <= c[1] && c[1] <= 0xBF) &&
                    (0x80 <= c[2] && c[2] <= 0xBF) &&
                    (0x80 <= c[3] && c[3] <= 0xBF)
                ) ||
                (// planes 4-15
                    (0xF1 <= c[0] && c[0] <= 0xF3) &&
                    (0x80 <= c[1] && c[1] <= 0xBF) &&
                    (0x80 <= c[2] && c[2] <= 0xBF) &&
                    (0x80 <= c[3] && c[3] <= 0xBF)
                ) ||
                (// plane 16
                    c[0] == 0xF4 &&
                    (0x80 <= c[1] && c[1] <= 0x8F) &&
                    (0x80 <= c[2] && c[2] <= 0xBF) &&
                    (0x80 <= c[3] && c[3] <= 0xBF)
                )
            ) 
          {
            c += 3;

            printf ("UTF8 [3]\n");
          }
          else
          {
            printf ("UTF8 [X] %c -> [%.1x]\n", *c, c[0]);
            
            ecstream_append_c (stream, *c);

            ecstream_append_c (stream, *c);  
          }
        }
      }
    }
  }
  else
  {
    ecstream_append_str (stream, "");
  }
}

//-----------------------------------------------------------------------------------------------------------

void jsonwriter_fill (EcStream stream, const EcUdc node)
{
//  printf ("fill [%p]\n", node);
  

  switch (ecudc_type (node))
  {
    case ENTC_UDC_LIST:
    {
      void* cursor = NULL;
      EcUdc item;
      uint_t counter = 0;
      
      ecstream_append_c (stream, '[');
      
      for (item  = ecudc_next (node, &cursor); isAssigned (item); item = ecudc_next (node, &cursor), counter++)
      {
        if (counter > 0)
        {
          ecstream_append_c (stream, ',');
        }
        jsonwriter_fill (stream, item);
      }
      
      ecstream_append_c (stream, ']');
    }
    break;
    case ENTC_UDC_NODE:
    {
      void* cursor = NULL;
      EcUdc item;
      uint_t counter = 0;
      
      ecstream_append_c (stream, '{');
      
      for (item  = ecudc_next (node, &cursor); isAssigned (item); item = ecudc_next (node, &cursor), counter++)
      {
        if (counter > 0)
        {
          ecstream_append_c (stream, ',');
        }
        ecstream_append_c (stream, '"');
        jsonwriter_escape (stream, ecudc_name (item));
        ecstream_append_str (stream, "\":");
        jsonwriter_fill (stream, item);
      }
      
      ecstream_append_c (stream, '}');
    }
    break;
    case ENTC_UDC_REF:
    {
      EcBuffer buffer = ecbuf_create (64);
      ecbuf_format (buffer, 64, "%p", ecudc_asP (node)); 
      
      ecstream_append_c (stream, '"');
      ecstream_append_str (stream, ecbuf_const_str (buffer));
      ecstream_append_c (stream, '"');
      
      ecbuf_destroy (&buffer);
    }
    break;
    case ENTC_UDC_STRING:
    {
      // check special chars at the beginning
      const EcString text = ecudc_asString (node);
      if (ecstr_leading (text, "##J"))
      {
        // already json encoded
        ecstream_append_str (stream, text + 3);
      }
      else
      {
        ecstream_append_c (stream, '"');
        jsonwriter_escape (stream, text);
        ecstream_append_c (stream, '"');
      }
    }
    break;
    case ENTC_UDC_BOOL:
    {
      ecstream_append_str (stream, ecudc_asBool (node) ? "true" : "false");
    }
    break;
    case ENTC_UDC_NONE:
    {
      ecstream_append_str (stream, "null");
    }
    break;
    case ENTC_UDC_NUMBER:
    {
      ecstream_append_i64 (stream, ecudc_asNumber (node));
    }
    break;
    case ENTC_UDC_TIME:
    {
      ecstream_append_time (stream, ecudc_asTime (node));
    }
    break;
    case ENTC_UDC_DOUBLE:
    {
      EcString h = ecstr_float (ecudc_asDouble(node), 10);
      
      ecstream_append_str (stream, h);
      
      ecstr_delete(&h);
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcBuffer ecjson_write (const EcUdc source)
{
  EcStream stream = ecstream_create ();
  
  jsonwriter_fill (stream, source);
  
  return ecstream_tobuf (&stream);
}

//-------------------------------------------------------------------------------------------------------

EcString ecjson_toString (const EcUdc source)
{
  EcBuffer buf = ecjson_write (source);
  
  return ecbuf_str (&buf);
}

//-------------------------------------------------------------------------------------------------------

int ecjson_readFromFile (const EcString filename, EcUdc* retUdc, const EcString secret, unsigned int sectype)
{
  int res;
  int bytesRead;

  EcFileHandle fh;
  EcErr err;
  EcJsonParser jparser;
  EcBuffer buf;
  EcDecryptAES aes = NULL;
  
  err = ecerr_create();

  // create crypto object
  if (secret)
  {
    aes = ecdecrypt_aes_create (secret, sectype, 0);
  }

  // open file
  fh = ecfh_open (filename, O_RDONLY);
  if (isNotAssigned (fh))
  {
    if (aes)
    {
      ecdecrypt_aes_destroy (&aes);
    }
    
    ecerr_destroy(&err);
    
    eclog_msg (LL_ERROR, "JSON", "reader", "can't open file");

    return ENTC_ERR_NOT_FOUND;
  }
  
  // create the parser
  jparser = ecjsonparser_create (ecjson_read_onItem, ecjson_read_onObjCreate, ecjson_read_onObjDestroy, NULL);
  
  // buffer for reading
  buf = ecbuf_create(10);
  
  for (bytesRead = ecfh_readBuffer (fh, buf); bytesRead > 0; bytesRead = ecfh_readBuffer (fh, buf))
  {
    EcBuffer_s h;
    
    h.buffer = buf->buffer;
    h.size = bytesRead;
    
    if (aes)
    {
      EcBuffer pbuf = ecdecrypt_aes_update (aes, &h, err);
      if (pbuf == NULL)
      {
        res = err->code;
        break;
      }
      
      h.buffer = pbuf->buffer;
      h.size = pbuf->size;
    }
    
    res = ecjsonparser_parse (jparser, (const char*)h.buffer, h.size, err);
    if (res)
    {
      eclog_fmt (LL_ERROR, "JSON", "reader", "can't parse: %s", err->text);
      eclog_msg (LL_WARN, "JSON", "reader", (char*)h.buffer);
      break;
    }
  }
  
  // finalize decryption
  if (aes && res == ENTC_ERR_NONE)
  {
    EcBuffer pbuf = NULL;
    
    pbuf = ecdecrypt_aes_finalize (aes, err);
    if (pbuf == NULL)
    {
      res = err->code;
    }
    
    res = ecjsonparser_parse (jparser, (const char*)pbuf->buffer, pbuf->size, err);
  }

  if (res == ENTC_ERR_NONE)
  {
    *retUdc = ecjsonparser_lastObject (jparser);
    
    if (*retUdc)
    {
      // set name
      ecudc_setName (*retUdc, NULL);
    }
  }

  // clean up
  ecjsonparser_destroy (&jparser);
  ecerr_destroy(&err);
  
  ecfh_close(&fh);
  ecbuf_destroy(&buf);
  
  if (aes)
  {
    ecdecrypt_aes_destroy (&aes);
  }
  
  return res;
}

//-------------------------------------------------------------------------------------------------------

int ecjson_writeToFile (const EcString filename, const EcUdc source, const EcString secret, unsigned int sectype)
{
  EcErr err = ecerr_create();
  EcFileHandle fh;
  int res = ENTC_ERR_NONE;
  EcEncryptAES aes = NULL;
  EcBuffer buf;

  if (secret)
  {
    aes = ecencrypt_aes_create (secret, sectype, 0);
  }
    
  fh = ecfh_open (filename, O_CREAT | O_RDWR | O_TRUNC);
  if (isNotAssigned (fh))
  {
    if (aes)
    {
      ecencrypt_aes_destroy (&aes);
    }
    
    ecerr_destroy(&err);
    
    return ENTC_ERR_NOT_FOUND;
  }

  // write ecudc to json text
  buf = ecjson_write (source);
  if (buf)
  {
    EcBuffer pbuf = buf;
    
    if (aes)
    {
      pbuf = ecencrypt_aes_update (aes, buf, err);
    }
    
    if (pbuf)
    {
      ecfh_writeBuffer (fh, pbuf, pbuf->size);
      pbuf = NULL;
    }

    if (aes)
    {
      pbuf = ecencrypt_aes_finalize (aes, err);
    }
    
    if (pbuf)
    {
      ecfh_writeBuffer (fh, pbuf, pbuf->size);
      pbuf = NULL;
    }
    
    // clean up
    ecbuf_destroy (&buf);
  }
  else
  {
    res = ENTC_ERR_PROCESS_FAILED;
  }
  
  ecfh_close(&fh);
  
  if (aes)
  {
    ecencrypt_aes_destroy (&aes);
  }

  ecerr_destroy (&err);
  
  return res;
}

//-------------------------------------------------------------------------------------------------------
