#include "entc_parser_json.h"

// cape includes
#include "stc/entc_stream.h"
#include "stc/entc_list.h"

// c includes
#include <stdlib.h>

//-----------------------------------------------------------------------------

#define JPARSER_STATE_NONE           0
#define JPARSER_STATE_KEY_BEG        1
#define JPARSER_STATE_LIST           2
#define JPARSER_STATE_KEY_RUN        3
#define JPARSER_STATE_KEY_END        4
#define JPARSER_STATE_VAL_BEG        5

#define JPARSER_STATE_STR_RUN        6
#define JPARSER_STATE_NODE_RUN       7
#define JPARSER_STATE_LIST_RUN       8
#define JPARSER_STATE_NUMBER_RUN     9
#define JPARSER_STATE_FLOAT_RUN     10
#define JPARSER_STATE_STR_ESCAPE    11
#define JPARSER_STATE_STR_UNICODE   12
#define JPARSER_STATE_KEY_ESCAPE    13
#define JPARSER_STATE_KEY_UNICODE   14

//=============================================================================

struct EntcParserJsonItem_s
{
  // original type
  int type;
  
  // original state
  int state;
  
  // for keeping the content
  EntcStream stream;
  
  // for counting an index
  int index;
  
  // for keeping the object
  void* obj;
  
  // callback
  EntcParserJson parser;
  fct_parser_json_onObjDel onDel;
  
}; typedef struct EntcParserJsonItem_s* EntcParserJsonItem;

//-----------------------------------------------------------------------------

EntcParserJsonItem entc_parser_json_item_new (int type, int state, EntcParserJson parser, fct_parser_json_onObjDel onDel)
{
  EntcParserJsonItem self = ENTC_NEW (struct EntcParserJsonItem_s);
  
  self->stream = entc_stream_new ();
  self->index = 0;
  self->type = type;
  self->state = state;
  self->obj = NULL;
  
  self->parser = parser;
  self->onDel = onDel;
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_parser_json_item_delLastObject (EntcParserJsonItem self)
{
  if (self->obj && self->onDel)
  {
    self->onDel (self->parser, self->obj);
    self->obj = NULL;
  }
}

//-----------------------------------------------------------------------------

void entc_parser_json_item_del (EntcParserJsonItem* p_self)
{
  EntcParserJsonItem self = *p_self;
  
  entc_stream_del (&(self->stream));
  
  entc_parser_json_item_delLastObject (self);
    
  ENTC_DEL (p_self, struct EntcParserJsonItem_s);
}

//-----------------------------------------------------------------------------

void entc_parser_json_item_setObject (EntcParserJsonItem self, void** obj, int type)
{
  entc_parser_json_item_delLastObject (self);
  
  // transfer object
  self->obj = *obj;
  *obj = NULL;
  
  self->type = type;
}

//-----------------------------------------------------------------------------

struct EntcParserJson_s
{
  fct_parser_json_onItem onItem;
  fct_parser_json_onObjNew onObjNew;
  fct_parser_json_onObjDel onObjDel;
  
  void* ptr;
  
  EntcList stack;
  
  EntcParserJsonItem keyElement;
  EntcParserJsonItem valElement;
  
  // for unicode decoding
  unsigned char unicode_data[10];
  int unicode_pos;
};

//-----------------------------------------------------------------------------

static void __STDCALL entc_parser_json_stack_onDel (void* ptr)
{
  EntcParserJsonItem item = ptr;
  
  entc_parser_json_item_del (&item);
}

//-----------------------------------------------------------------------------

static void __STDCALL entc_parser_json_element_push_onObjDel (void* ptr, void* obj)
{
  EntcParserJson self = ptr;
  
  if (self->onObjDel)
  {
    self->onObjDel (self->ptr, obj);
  }
}

//-----------------------------------------------------------------------------

EntcParserJson entc_parser_json_new (void* ptr, fct_parser_json_onItem onItem, fct_parser_json_onObjNew onObjNew, fct_parser_json_onObjDel onObjDel)
{
  EntcParserJson self = ENTC_NEW (struct EntcParserJson_s);
  
  self->ptr = ptr;
  self->onItem = onItem;
  self->onObjNew = onObjNew;
  self->onObjDel = onObjDel;
  
  self->keyElement = NULL; // not yet created
  self->valElement = entc_parser_json_item_new (ENTC_JPARSER_UNDEFINED, JPARSER_STATE_NONE, self, entc_parser_json_element_push_onObjDel);
  
  self->stack = entc_list_new (entc_parser_json_stack_onDel);
  
  // prepare the hex notation 0x0000
  self->unicode_data [0] = '0';
  self->unicode_data [1] = 'x';
  self->unicode_data [2] = '\0';
  self->unicode_data [3] = '\0';
  self->unicode_data [4] = '\0';
  self->unicode_data [5] = '\0';
  self->unicode_data [6] = '\0';
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_parser_json_del (EntcParserJson* p_self)
{
  EntcParserJson self = *p_self;
  
  entc_list_del (&(self->stack));
  
  entc_parser_json_item_del (&(self->valElement));
    
  ENTC_DEL (p_self, struct EntcParserJson_s);
}

//-----------------------------------------------------------------------------

void entc_parser_json_push (EntcParserJson self, int type, int state)
{
  EntcParserJsonItem item = entc_parser_json_item_new (type, state, self, entc_parser_json_element_push_onObjDel);
  
  if (self->onObjNew)
  {
    item->obj = self->onObjNew (self->ptr, type);
  }
  
  entc_list_push_back (self->stack, item);
  
  self->keyElement = item;
}

//-----------------------------------------------------------------------------

void entc_parser_json_pop (EntcParserJson self)
{
  // transfer all element values to the content element
  {
    EntcParserJsonItem element = entc_list_pop_back (self->stack);
    
    entc_parser_json_item_setObject (self->valElement, &(element->obj), element->type);
    
    entc_parser_json_stack_onDel (element);
  }
  
  // fetch last element
  {
    EntcListCursor cursor;
    entc_list_cursor_init (self->stack, &cursor, ENTC_DIRECTION_FORW);
    
    if (entc_list_cursor_next (&cursor))
    {
      self->keyElement = entc_list_node_data (cursor.node);
    }
    else
    {
      self->keyElement = NULL;
    }
  }
}

//-----------------------------------------------------------------------------

int entc_parser_json_leave_value (EntcParserJson self, int state)
{
  if (state == JPARSER_STATE_LIST_RUN)
  {
    entc_stream_clr (self->valElement->stream);
    return JPARSER_STATE_VAL_BEG;
  }
  else if (state == JPARSER_STATE_NODE_RUN)
  {
    return JPARSER_STATE_KEY_BEG;
  }
  
  return JPARSER_STATE_NONE;
}

//-----------------------------------------------------------------------------

/*
void entc_parser_json_decode_unicode (unsigned int unicode, EntcStream dest)
{
  if (unicode < 0x80)
  {
    entc_stream_append_c (dest, unicode);
  }
  else if (unicode < 0x800)
  {
    entc_stream_append_c (dest, 192 + unicode / 64);
    entc_stream_append_c (dest, 128 + unicode % 64);
  }
  else if (unicode - 0xd800u < 0x800)
  {
    // error
  }
  else if (unicode < 0x10000)
  {
    entc_stream_append_c (dest, 224 + unicode / 4096);
    entc_stream_append_c (dest, 128 + unicode /64 % 64);
    entc_stream_append_c (dest, 128 + unicode % 64);
  }
  else if (unicode < 0x110000)
  {
    entc_stream_append_c (dest, 240 + unicode / 262144);
    entc_stream_append_c (dest, 128 + unicode / 4096 % 64);
    entc_stream_append_c (dest, 128 + unicode / 64 % 64);
    entc_stream_append_c (dest, 128 + unicode % 64);
  }
  else
  {
    // error
  }
}

//-----------------------------------------------------------------------------

unsigned int entc_parser_json_decode_unicode_point (const char** pc)
{
  int i;
  const char* pos = *pc;
  unsigned int unicode = 0;
  
  for (i = 0; (i < 4) && *pos; ++i, ++pos)
  {
    char c = *pos;
    unicode *= 16;
    
    if ( c >= '0'  &&  c <= '9' )
      unicode += c - '0';
    else if ( c >= 'a'  &&  c <= 'f' )
      unicode += c - 'a' + 10;
    else if ( c >= 'A'  &&  c <= 'F' )
      unicode += c - 'A' + 10;
    else
    {
      // "Bad unicode escape sequence in string: hexadecimal digit expected."
      return 0;
    }
  }
  
  *pc = pos - 1;
  
  return unicode;
}
*/

//-----------------------------------------------------------------------------

void entc_parser_json_decode_unicode_hex (EntcParserJson self, EntcStream dest)
{
  // convert from HEX into wide char
  wchar_t wc = strtol ((const char*)self->unicode_data, NULL, 16);
  
  //printf ("HEX: %s -> %u\n", self->unicode_data, wc);
  
  if ( 0 <= wc && wc <= 0x7f )
  {
    entc_stream_append_c (dest, wc);
  }
  else if ( 0x80 <= wc && wc <= 0x7ff )
  {
    entc_stream_append_c (dest, 0xc0 | (wc >> 6));
    entc_stream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x800 <= wc && wc <= 0xffff )
  {
    entc_stream_append_c (dest, 0xe0 | (wc >> 12));
    entc_stream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f));
    entc_stream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x10000 <= wc && wc <= 0x1fffff )
  {
    entc_stream_append_c (dest, 0xf0 | (wc >> 18));
    entc_stream_append_c (dest, 0x80 | ((wc >> 12) & 0x3f));
    entc_stream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f));
    entc_stream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x200000 <= wc && wc <= 0x3ffffff )
  {
    entc_stream_append_c (dest, 0xf8 | (wc >> 24));
    entc_stream_append_c (dest, 0x80 | ((wc >> 18) & 0x3f));
    entc_stream_append_c (dest, 0x80 | ((wc >> 12) & 0x3f));
    entc_stream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f));
    entc_stream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x4000000 <= wc && wc <= 0x7fffffff )
  {
    entc_stream_append_c (dest, 0xfc | (wc >> 30) );
    entc_stream_append_c (dest, 0x80 | ((wc >> 24) & 0x3f) );
    entc_stream_append_c (dest, 0x80 | ((wc >> 18) & 0x3f) );
    entc_stream_append_c (dest, 0x80 | ((wc >> 12) & 0x3f) );
    entc_stream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f) );
    entc_stream_append_c (dest, 0x80 | (wc & 0x3f) );
  }
}

//-----------------------------------------------------------------------------

void entc_parser_json_item_next (EntcParserJson self, int type, const char* key, int index)
{
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_TEXT:
    {
      if (self->onItem)
      {
        // void* ptr, void* obj, int type, const char* key, void* val
        self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_TEXT, (void*)entc_stream_get (self->valElement->stream), key, index);
      }
      
      entc_stream_clr (self->valElement->stream);
      break;
    }
    case ENTC_JPARSER_OBJECT_NUMBER:
    {
      if (self->onItem)
      {
        char* endptr = NULL;
        const char* val = entc_stream_get (self->valElement->stream);
        
#ifdef _WIN32
        number_t dat = atoi (val);

        self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_NUMBER, (void*)&dat, key, index);
#else
        number_t dat = strtoll (val, &endptr, 10);
        if (endptr == NULL)
        {
          // was not able to transform
          
        }
        else
        {
          // void* ptr, void* obj, int type, const char* key, void* val
          self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_NUMBER, (void*)&dat, key, index);
        }        
#endif
      }
      
      entc_stream_clr (self->valElement->stream);
      break;
    }
    case ENTC_JPARSER_OBJECT_FLOAT:
    {      
      if (self->onItem)
      {
        char* endptr = NULL;
        const char* val = entc_stream_get (self->valElement->stream);
        
        double dat = strtod (val, &endptr);
        if (endptr == NULL)
        {
          // was not able to transform
          
        }
        else
        {
          // void* ptr, void* obj, int type, const char* key, void* val
          self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_FLOAT, (void*)&dat, key, index);
        }
      }
      
      entc_stream_clr (self->valElement->stream);
      break;
    }
    case ENTC_JPARSER_UNDEFINED:
    {
      switch (self->valElement->type)
      {
        case ENTC_JPARSER_OBJECT_NODE:
        case ENTC_JPARSER_OBJECT_LIST:
        {
          if (self->onItem)
          {
            self->onItem (self->ptr, self->keyElement->obj, self->valElement->type, self->valElement->obj, key, index);
            
            // assume that the object was transfered
            self->valElement->obj = NULL;
            self->valElement->type = ENTC_JPARSER_UNDEFINED;
          }
          
          break;
        }
        case ENTC_JPARSER_UNDEFINED:
        {
          const char* val = entc_stream_get (self->valElement->stream);
          
          // we need to detect the kind of element
          switch (val[0])
          {
            case 't':
            {
              if (strcmp("true", val) == 0 && self->onItem)
              {
                self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_BOLEAN, (void*)TRUE, key, index);
              }
              
              entc_stream_clr (self->valElement->stream);
              
              break;
            }
            case 'f':
            {
              if (strcmp("false", val) == 0 && self->onItem)
              {
                self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_BOLEAN, (void*)FALSE, key, index);
              }
              
              entc_stream_clr (self->valElement->stream);
              
              break;
            }
            case 'n':
            {
              if (strcmp("null", val) == 0 && self->onItem)
              {
                self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_NULL, NULL, key, index);
              }
              
              entc_stream_clr (self->valElement->stream);
              
              break;
            }
          }
          
          entc_stream_clr (self->valElement->stream);
          
          break;
        }
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------

int entc_parser_json_item (EntcParserJson self, int type)
{
  if (self->keyElement)
  {
    switch (self->keyElement->state)
    {
      case JPARSER_STATE_NODE_RUN:
      {
        entc_parser_json_item_next (self, type, entc_stream_get (self->keyElement->stream), 0);
        
        entc_stream_clr (self->keyElement->stream);
        break;
      }
      case JPARSER_STATE_LIST_RUN:
      {
        entc_parser_json_item_next (self, type, NULL, self->keyElement->index);
        
        self->keyElement->index++;
        break;
      }
    }
    
    return self->keyElement->state;
  }
  
  return JPARSER_STATE_NONE;
}

//-----------------------------------------------------------------------------

int entc_parser_json_process (EntcParserJson self, const char* buffer, number_t size, EntcErr err)
{
  const char* c = buffer;
  number_t i;
  
  int state = self->valElement->state;
  
  for (i = 0; (i < size) && *c; i++, c++)
  {
    switch (*c)
    {
      case '{':
      {
        switch (state)
        {
          case JPARSER_STATE_NONE:
          {
            entc_parser_json_push (self, ENTC_JPARSER_OBJECT_NODE, JPARSER_STATE_NODE_RUN);
            
            state = JPARSER_STATE_KEY_BEG;
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            entc_parser_json_push (self, ENTC_JPARSER_OBJECT_NODE, JPARSER_STATE_NODE_RUN);
            
            state = JPARSER_STATE_KEY_BEG;
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '{'");
          }
        }
        
        break;
      }
      case '[':
      {
        switch (state)
        {
          case JPARSER_STATE_NONE:
          {
            state = JPARSER_STATE_VAL_BEG;
            entc_stream_clr (self->valElement->stream);
            
            entc_parser_json_push (self, ENTC_JPARSER_OBJECT_LIST, JPARSER_STATE_LIST_RUN);
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = JPARSER_STATE_VAL_BEG;
            entc_stream_clr (self->valElement->stream);
            
            entc_parser_json_push (self, ENTC_JPARSER_OBJECT_LIST, JPARSER_STATE_LIST_RUN);
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '['");
          }
        }
        
        break;
      }
      case '}':
      {
        switch (state)
        {
          case JPARSER_STATE_NODE_RUN:
          {
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_KEY_BEG:
          {
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_UNDEFINED);
            
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_NUMBER);
            
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_FLOAT);
            
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '}'");
          }
        }
        break;
      }
      case ']':
      {
        switch (state)
        {
          case JPARSER_STATE_LIST_RUN:
          {
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_UNDEFINED);
            
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_NUMBER);
            
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_FLOAT);
            
            entc_parser_json_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in ']'");
          }
        }
        break;
      }
      case '"':
      {
        switch (state)
        {
          case JPARSER_STATE_KEY_BEG:    // start of key
          {
            state = JPARSER_STATE_KEY_RUN;
            break;
          }
          case JPARSER_STATE_KEY_RUN:    // end of key
          {
            state = JPARSER_STATE_KEY_END;
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = JPARSER_STATE_STR_RUN;
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_TEXT);
            break;
          }
          case JPARSER_STATE_KEY_ESCAPE:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            state = JPARSER_STATE_KEY_RUN;
            break;
          }
          case JPARSER_STATE_STR_ESCAPE:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            state = JPARSER_STATE_STR_RUN;
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '\"'");
          }
        }
        break;
      }
      case '\\':
      {
        switch (state)
        {
          case JPARSER_STATE_KEY_RUN:
          {
            state = JPARSER_STATE_KEY_ESCAPE;
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            state = JPARSER_STATE_STR_ESCAPE;
            break;
          }
          case JPARSER_STATE_KEY_ESCAPE:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            state = JPARSER_STATE_KEY_RUN;
            break;
          }
          case JPARSER_STATE_STR_ESCAPE:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            state = JPARSER_STATE_STR_RUN;
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '\\'");
            
          }
        }
        break;
      }
      case ',':
      {
        switch (state)
        {
          case JPARSER_STATE_NODE_RUN:
          {
            state = JPARSER_STATE_KEY_BEG;
            break;
          }
          case JPARSER_STATE_LIST_RUN:
          {
            state = JPARSER_STATE_VAL_BEG;
            entc_stream_clr (self->valElement->stream);
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_UNDEFINED);
            
            state = entc_parser_json_leave_value (self, state);
            
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_NUMBER);
            
            state = entc_parser_json_leave_value (self, state);
            
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            state = entc_parser_json_item (self, ENTC_JPARSER_OBJECT_FLOAT);
            
            state = entc_parser_json_leave_value (self, state);
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_KEY_RUN:    // end of key
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in ','");
          }
        }
        break;
      }
      case ':':
      {
        switch (state)
        {
          case JPARSER_STATE_KEY_END:
          {
            state = JPARSER_STATE_VAL_BEG;
            entc_stream_clr (self->valElement->stream);
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in ':'");
          }
        }
        break;
      }
      case ' ':
      case '\n':
      case '\r':
      case '\t':
      {
        switch (state)
        {
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            // ignore
          }
        }
        
        break;
      }
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      {
        switch (state)
        {
          case JPARSER_STATE_VAL_BEG:
          {
            state = JPARSER_STATE_NUMBER_RUN;
            
            entc_stream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_KEY_RUN:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_KEY_UNICODE:
          {
            self->unicode_data [self->unicode_pos + 2] = *c;
            self->unicode_pos++;
            
            if (self->unicode_pos == 4)
            {
              EntcParserJsonItem element = self->keyElement;
              
              if (element)
              {
                entc_parser_json_decode_unicode_hex (self, element->stream);
              }
              
              state = JPARSER_STATE_KEY_RUN;
            }
            
            break;
          }
          case JPARSER_STATE_STR_UNICODE:
          {
            self->unicode_data [self->unicode_pos + 2] = *c;
            self->unicode_pos++;
            
            if (self->unicode_pos == 4)
            {
              entc_parser_json_decode_unicode_hex (self, self->valElement->stream);              
              state = JPARSER_STATE_STR_RUN;
            }
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state by number");
          }
        }
        
        break;
      }
      case '-':
      {
        switch (state)
        {
          case JPARSER_STATE_VAL_BEG:
          {
            state = JPARSER_STATE_NUMBER_RUN;
            
            entc_stream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_KEY_RUN:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '-'");
          }
        }
        
        break;
      }
      case '.':
      {
        switch (state)
        {
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = JPARSER_STATE_FLOAT_RUN;
            
            entc_stream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_KEY_RUN:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set (err, ENTC_ERR_PARSER, "unexpected state in '.'");
          }
        }
        
        break;
      }
      default:
      {
        switch (state)
        {
          case JPARSER_STATE_KEY_RUN:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element)
            {
              entc_stream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_KEY_ESCAPE:
          {
            EntcParserJsonItem element = self->keyElement;
            
            if (element) switch (*c)
            {
              case '/':
              {
                entc_stream_append_c (element->stream, '/');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'n':
              {
                entc_stream_append_c (element->stream, '\n');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 't':
              {
                entc_stream_append_c (element->stream, '\t');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'r':
              {
                entc_stream_append_c (element->stream, '\r');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'b':
              {
                entc_stream_append_c (element->stream, '\b');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'f':
              {
                entc_stream_append_c (element->stream, '\f');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'u':
              {
                // hex encoded unicode
                state = JPARSER_STATE_KEY_UNICODE;
                self->unicode_pos = 0;
                
                break;
              }
              default:
              {
//                eclog_fmt (LL_WARN, "CAPE", "ecjparser", "unknown escape sequence '%c' -> [%i]", *c, *c);
                
                entc_stream_append_c (element->stream, *c);
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
            }
            break;
          }
          case JPARSER_STATE_STR_ESCAPE:
          {
            // check excape sequence
            switch (*c)
            {
              case '/':
              {
                entc_stream_append_c (self->valElement->stream, '/');
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'n':
              {
                entc_stream_append_c (self->valElement->stream, '\n');
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 't':
              {
                entc_stream_append_c (self->valElement->stream, '\t');
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'r':
              {
                entc_stream_append_c (self->valElement->stream, '\r');
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'b':
              {
                entc_stream_append_c (self->valElement->stream, '\b');
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'f':
              {
                entc_stream_append_c (self->valElement->stream, '\f');
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'u':
              {
                // hex encoded unicode
                state = JPARSER_STATE_STR_UNICODE;                
                self->unicode_pos = 0;
                
                break;
              }
              default:
              {
                //eclog_fmt (LL_WARN, "CAPE", "ecjparser", "unknown escape sequence '%c' -> [%i]", *c, *c);
                
                entc_stream_append_c (self->valElement->stream, *c);
                
                state = JPARSER_STATE_STR_RUN;
                break;
              }
            }
            
            break;
          }
          case JPARSER_STATE_KEY_UNICODE:
          {
            self->unicode_data [self->unicode_pos + 2] = *c;
            self->unicode_pos++;
            
            if (self->unicode_pos == 4)
            {
              EntcParserJsonItem element = self->keyElement;
              
              if (element)
              {
                entc_parser_json_decode_unicode_hex (self, element->stream);
              }
              
              state = JPARSER_STATE_KEY_RUN;
            }
            
            break;
          }
          case JPARSER_STATE_STR_UNICODE:
          {
            self->unicode_data [self->unicode_pos + 2] = *c;
            self->unicode_pos++;
            
            if (self->unicode_pos == 4)
            {
              entc_parser_json_decode_unicode_hex (self, self->valElement->stream);
              state = JPARSER_STATE_STR_RUN;
            }
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            entc_stream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return entc_err_set_fmt (err, ENTC_ERR_PARSER, "unexpected state [%i] in default '%c' <- [%i]", state, *c, *c);
          }
        }
        
        break;
      }
    }
  }
  
  self->valElement->state = state;
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void* entc_parser_json_object (EntcParserJson self)
{
  void* obj = self->valElement->obj;
  
  self->valElement->obj = NULL;
  
  return obj;
}

//-----------------------------------------------------------------------------
