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

EcUdc json_parse_type (JsonParser* parser, const EcString key, int len)
{
  EcUdc udc = NULL;
  
  EcString h1 = ecstr_part ((const char*)parser->pos + 2, len);
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
    udc = ecudc_create(EC_ALLOC, ENTC_UDC_INT32, key);    
    ecudc_setInt32(udc, atoi(h2));
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
          // todo
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
          EcUdc h = json_parse_type (parser, key, c - parser->pos - 2);
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
          EcUdc h = json_parse_type (parser, key, c - parser->pos - 2);
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

EcUdc ecjson_read (const EcString source, const EcString name)
{
  JsonParser parser;
  
  parser.pos = (unsigned char*)source;
  
  return json_parse (&parser, name);  
}

//-----------------------------------------------------------------------------------------------------------

void jsonwriter_escape (EcStream stream, const EcString source)
{
  if (ecstr_valid (source))
  {
    const char* c;
    for (c = source; *c; c++)
    {
      if (*c == '"')
      {
        ecstream_appendc (stream, '\\');
        ecstream_appendc (stream, *c);
      }
      else if (*c <= 0x7e)
      {
        ecstream_appendc (stream, *c);
      }
    }
  }
  else
  {
    ecstream_append (stream, "");    
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
      
      ecstream_append (stream, "[");
      
      for (item  = ecudc_next (node, &cursor); isAssigned (item); item = ecudc_next (node, &cursor), counter++)
      {
        if (counter > 0)
        {
          ecstream_appendc (stream, ',');
        }
        jsonwriter_fill (stream, item);
      }
      
      ecstream_appendc (stream, ']');     
    }
    break;
    case ENTC_UDC_NODE:
    {
      void* cursor = NULL;
      EcUdc item;
      uint_t counter = 0;
      
      ecstream_append (stream, "{");
      
      for (item  = ecudc_next (node, &cursor); isAssigned (item); item = ecudc_next (node, &cursor), counter++)
      {
        if (counter > 0)
        {
          ecstream_appendc (stream, ',');
        }
        ecstream_appendc (stream, '"');
        jsonwriter_escape (stream, ecudc_name (item));
        ecstream_append (stream, "\":");
        jsonwriter_fill (stream, item);
      }
      
      ecstream_appendc (stream, '}');     
    }
    break;
    case ENTC_UDC_REF:
    {
      EcBuffer buffer = ecbuf_create (64);
      ecbuf_format (buffer, 64, "%p", ecudc_asP (node)); 
      
      ecstream_appendc (stream, '"');
      ecstream_append (stream, ecbuf_const_str (buffer));
      ecstream_appendc (stream, '"');
      
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
        ecstream_append (stream, text + 3);
      }
      else
      {
        ecstream_appendc (stream, '"');
        jsonwriter_escape (stream, text);
        ecstream_appendc (stream, '"');        
      }
    }
    break;
    case ENTC_UDC_BOOL:
    {
      ecstream_append (stream, ecudc_asBool (node) ? "true" : "false");
    }
    break;
    case ENTC_UDC_NONE:
    {
      ecstream_append (stream, "null");
    }
    break;
    case ENTC_UDC_BYTE:
    {
      ecstream_appends (stream, ecudc_asByte (node));
    }
    break;
    case ENTC_UDC_UBYTE:
    {
      ecstream_appendu (stream, ecudc_asUByte (node));
    }
    break;
    case ENTC_UDC_INT16:
    {
      ecstream_appends (stream, ecudc_asInt16 (node));
    }
    break;
    case ENTC_UDC_UINT16:
    {
      ecstream_appendu (stream, ecudc_asUInt16 (node));
    }
    break;
    case ENTC_UDC_INT32:
    {
      ecstream_appends (stream, ecudc_asInt32 (node));
    }
    break;
    case ENTC_UDC_UINT32:
    {
      ecstream_appendu (stream, ecudc_asUInt32 (node));
    }
    break;
    case ENTC_UDC_INT64:
    {
      ecstream_appends (stream, ecudc_asInt64 (node));
    }
    break;
    case ENTC_UDC_UINT64:
    {
      ecstream_appendu (stream, ecudc_asUInt64 (node));
    }
    break;
    case ENTC_UDC_TIME:
    {
      ecstream_appendt (stream, ecudc_asTime (node));
    }
    break;
    case ENTC_UDC_DOUBLE:
    {
      EcString h = ecstr_float (ecudc_asDouble(node), 10);
      
      ecstream_append (stream, h);
      
      ecstr_delete(&h);
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------------------------

EcString ecjson_write (const EcUdc source)
{
  EcBuffer buffer;
  
  EcStream stream = ecstream_new ();
  
  jsonwriter_fill (stream, source);
  
  buffer = ecstream_trans (&stream);
  
  return ecbuf_str (&buffer);  
}

//-----------------------------------------------------------------------------------------------------------
