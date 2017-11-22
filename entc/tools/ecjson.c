/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#include "ecjson.h"
#include "types/ecstream.h"
#include "utils/eclogger.h"

#include "ecjparser.h"

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
            eclogger_msg (LL_TRACE, "JSON", "reader", "array already assigned");
            
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
            eclogger_msg (LL_TRACE, "JSON", "reader", "array already assigned");
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
      
      int dat = (int)val;
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

EcUdc ecjson_readFromBuffer (const EcBuffer buf, const EcString name)
{
  EcUdc ret = NULL;
  int res;

  EcErr err = ecerr_create();
  EcJsonParser jparser = ecjsonparser_create (ecjson_read_onItem, ecjson_read_onObjCreate, ecjson_read_onObjDestroy, NULL);
  
  res = ecjsonparser_parse (jparser, (const char*)buf->buffer, buf->size, err);
  if (res)
  {
    eclogger_msg (LL_ERROR, "JSON", "reader", err->text);

    eclogger_msg (LL_WARN, "JSON", "%s", (const char*)buf->buffer);
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

EcUdc ecjson_read (const EcString source, const EcString name)
{
  EcUdc ret = NULL;
  int res;
 
  if (source)
  {
    EcErr err = ecerr_create();
    
    EcJsonParser jparser = ecjsonparser_create (ecjson_read_onItem, ecjson_read_onObjCreate, ecjson_read_onObjDestroy, NULL);
    
    res = ecjsonparser_parse (jparser, source, strlen(source), err);
    if (res)
    {
      eclogger_msg (LL_ERROR, "JSON", "reader", err->text);
      
      eclogger_msg (LL_WARN, "JSON", "%s", source);
    }
    else
    {
      ret = ecjsonparser_lastObject(jparser);
    }
    
    if (ret)
    {
      // set name
      ecudc_setName (ret, name);
    }
    
    // clean up
    ecjsonparser_destroy (&jparser);
    ecerr_destroy(&err);
  }

  return ret;
}

//-----------------------------------------------------------------------------------------------------------

void jsonwriter_escape (EcStream stream, const EcString source)
{
  if (ecstr_valid (source))
  {
    const char* c;
    for (c = source; *c; c++)
    {
      switch (*c)
      {
        case '"':
        case '\\':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, *c);
        }
        break;
        case '\r':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 'r');
        }
        break;
        case '\n':
        {
          ecstream_append_c (stream, '\\');
          ecstream_append_c (stream, 'n');
        }
        break;
        default:
        {
          if (*c <= 0x7e)
          {
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

EcString ecjson_write (const EcUdc source)
{
  EcBuffer buffer;
  
  EcStream stream = ecstream_create ();
  
  jsonwriter_fill (stream, source);
  
  buffer = ecstream_tobuf (&stream);
  
  return ecbuf_str (&buffer);  
}

//-------------------------------------------------------------------------------------------------------

int ecjson_readFromFile (const EcString filename, EcUdc* retUdc)
{
  uint64_t fsize;
  EcBuffer content;
  EcFileHandle fh;

  fh = ecfh_open (filename, O_RDONLY);
  if (isNotAssigned (fh))
  {
    return ENTC_RESCODE_NOT_AVAILABLE;
  }
  
  fsize = ecfh_size (fh);
  
  // TODO: using stream
  content = ecbuf_create (fsize + 1);
  
  if (ecfh_readBuffer (fh, content) == 0)
  {
    return ENTC_RESCODE_NOT_AVAILABLE;
  }
  
  ecfh_close(&fh);
  
  {
    EcString text = ecbuf_str (&content);
  
    *retUdc = ecjson_read(text, NULL);
  
    ecstr_delete (&text);
  }  

  return ENTC_RESCODE_OK;
}

//-------------------------------------------------------------------------------------------------------

int ecjson_writeToFile (const EcString filename, const EcUdc source)
{
  EcFileHandle fh;
  int res = ENTC_RESCODE_OK;

  fh = ecfh_open (filename, O_CREAT | O_RDWR | O_TRUNC);
  if (isNotAssigned (fh))
  {
    return ENTC_RESCODE_NOT_AVAILABLE;
  }
  
  {
    EcString text = ecjson_write (source);
    if (text)
    {
      ecfh_writeString (fh, text);
      
      // clean up
      ecstr_delete (&text);
    }
    else
    {
      res = ENTC_RESCODE_NOT_AVAILABLE;
    }
  }
  
  ecfh_close(&fh);
  return res;
}

//-------------------------------------------------------------------------------------------------------
