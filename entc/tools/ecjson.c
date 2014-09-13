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

//-----------------------------------------------------------------------------------------------------------

#define STATE_NONE 0
#define STATE_OBJECT 1
#define STATE_ARRAY 2

typedef struct {
  
  const unsigned char* pos;
  
  EcLogger logger;
  
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

#define ENT_STATE_NONE            0
#define ENT_STATE_ARRAY           1
#define ENT_STATE_ARRAY_EOE       2
#define ENT_STATE_OBJECT          3
#define ENT_STATE_OBJECT_KEY      4
#define ENT_STATE_OBJECT_KEY_EOE  5
#define ENT_STATE_OBJECT_EOE      6

// implemented as state machine
EcUdc json_parse (JsonParser* parser, const char* name)
{
  int state = ENT_STATE_NONE;
  const unsigned char* c;  
  EcUdc udc = NULL;
  EcString key = NULL;
  
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
            eclogger_log (parser->logger, LL_TRACE, "JSON", "{reader} array already assigned");
            // error
            ecudc_destroy(&udc);
            
            return NULL;
          }
          
          udc = ecudc_create (ENTC_UDC_LIST, name); 
          
          parser->pos = c;
          state = ENT_STATE_ARRAY;
        }
          break;
        case '{':
        {
          if (isAssigned (udc))
          {
            eclogger_log (parser->logger, LL_TRACE, "JSON", "{reader} array already assigned");
            // error
            ecudc_destroy(&udc);
            
            return NULL;
          }
          
          udc = ecudc_create (ENTC_UDC_NODE, name); 
          
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
              return NULL;
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
              return NULL;
            }
            ecudc_add_asString(udc, "", value);
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
          return NULL;
      }
        break;
      case ENT_STATE_OBJECT_KEY: switch (*c)
      {
        case '\"':
        {
          parser->pos = c + 1;
          {
            key = json_parse_string (parser);
            if (isNotAssigned (key))
            {
              return NULL;
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
          return NULL;          
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
          return NULL;
      }
        break;
      case ENT_STATE_OBJECT: switch (*c)
      {
        case '}':
        {
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
              return NULL;
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
              return NULL;
            }
            ecudc_add_asString(udc, key, value);
            ecstr_delete(&value);
          }
          c = parser->pos;
          state = ENT_STATE_OBJECT_EOE;
        }
          break;
        case ',':
        {
          // todo
          state = ENT_STATE_OBJECT_KEY;
          parser->pos = c;
        }
          break;
          // ignore
        case '\t' : case '\r' : case '\n' : case ' ': 
          break;
      }
        break;
      case ENT_STATE_OBJECT_EOE: switch (*c)
      {
        case '}':
        {
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
          return NULL;
      }
        break;
    }
  }
  return NULL;
}

//-----------------------------------------------------------------------------------------------------------

EcUdc ecjson_read (const EcString source, const EcString name, EcLogger logger)
{
  JsonParser parser;
  
  parser.pos = (unsigned char*)source;
  parser.logger = logger;
  
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
      if (*c <= 0x7e)
      {
        ecstream_appendc (stream, *c);
      }
      else
      {
        
      }
    }
  }
  else
  {
    ecstream_append (stream, "null");    
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
      ecstream_appendc (stream, '"');
      jsonwriter_escape (stream, ecudc_asString (node));
      ecstream_appendc (stream, '"');
    }
      break;
    case ENTC_UDC_BYTE:
    {
      ecstream_appendu (stream, ecudc_asB (node));
    }
      break;
    case ENTC_UDC_UINT32:
    {
      ecstream_appendu (stream, ecudc_asUInt32 (node));
    }
      break;
    case ENTC_UDC_UINT64:
    {
      ecstream_appendu (stream, ecudc_asUInt64 (node));
    }
      break;
    case ENTC_UDC_TIME:
    {
      ecstream_appendu (stream, ecudc_asTime (node));
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
