
#include "ecjparser.h"

// entc include
#include "system/macros.h"
#include "types/ecstream.h"
#include "types/eclist.h"
#include "tools/eclog.h"

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

typedef struct
{
  // original type
  int type;
  
  // original state
  int state;
  
  // for keeping the content
  EcStream stream;
  
  // for counting an index
  int index;
  
  // for keeping the object
  void* obj;
  
  // callback
  EcJsonParser parser;
  fct_ecjparser_onObjDestroy onDestroy;
  
} EcJsonParserItem;

//-----------------------------------------------------------------------------

EcJsonParserItem* ecjsonparser_item_create (int type, int state, EcJsonParser parser, fct_ecjparser_onObjDestroy onDestroy)
{
  EcJsonParserItem* self = ENTC_NEW(EcJsonParserItem);
  
  self->stream = ecstream_create();
  self->index = 0;
  self->type = type;
  self->state = state;
  self->obj = NULL;
    
  self->parser = parser;
  self->onDestroy = onDestroy;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecjsonparser_item_destroy (EcJsonParserItem** pself)
{
  EcJsonParserItem* self = *pself;
  
  ecstream_destroy (&(self->stream));
  
  if (self->obj && self->onDestroy)
  {
    self->onDestroy (self->parser, self->obj);
    self->obj = NULL;
  }
  
  ENTC_DEL (pself, EcJsonParserItem);
}

//-----------------------------------------------------------------------------

void ecjsonparser_item_setObject (EcJsonParserItem* self, void** obj, int type)
{
  if (self->obj && self->onDestroy)
  {
    self->onDestroy (self->parser, self->obj);
    self->obj = NULL;
  }
  
  // transfer object
  self->obj = *obj;
  *obj = NULL;
  
  self->type = type;
}

//=============================================================================

struct EcJsonParser_s
{
  fct_ecjparser_onItem onItem;
  fct_ecjparser_onObjCreate onCreate;
  fct_ecjparser_onObjDestroy onDestroy;
  
  void* ptr;
  
  EcList stack;
  
  EcJsonParserItem* keyElement;
  EcJsonParserItem* valElement;
  
  // for unicode decoding
  unsigned char unicode_data[10];
  int unicode_pos;
};

//-----------------------------------------------------------------------------

static int __STDCALL ecjsonparser_create_stack_onDestroy (void* ptr)
{
  EcJsonParserItem* item = ptr;
  
  ecjsonparser_item_destroy (&item);
  
  return 0;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecjsonparser_element_push_onObjDestroy (void* ptr, void* obj)
{
  EcJsonParser self = ptr;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr, obj);
  }
}

//-----------------------------------------------------------------------------

EcJsonParser ecjsonparser_create (fct_ecjparser_onItem onItem, fct_ecjparser_onObjCreate onCreate, fct_ecjparser_onObjDestroy onDestroy, void* ptr)
{
  EcJsonParser self = ENTC_NEW(struct EcJsonParser_s);
  
  self->ptr = ptr;
  self->onItem = onItem;
  self->onCreate = onCreate;
  self->onDestroy = onDestroy;
  
  self->keyElement = NULL; // not yet created
  self->valElement = ecjsonparser_item_create (ENTC_JPARSER_UNDEFINED, JPARSER_STATE_NONE, self, ecjsonparser_element_push_onObjDestroy);
  
  self->stack = eclist_create (ecjsonparser_create_stack_onDestroy);
  
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

void ecjsonparser_destroy (EcJsonParser* pself)
{
  EcJsonParser self = *pself;
  
  eclist_destroy (&(self->stack));
  
  ecjsonparser_item_destroy (&(self->valElement));
  
  ENTC_DEL (pself, struct EcJsonParser_s);
}

//-----------------------------------------------------------------------------

void ecjsonparser_push (EcJsonParser self, int type, int state)
{
  EcJsonParserItem* item = ecjsonparser_item_create (type, state, self, ecjsonparser_element_push_onObjDestroy);
  
  if (self->onCreate)
  {
    item->obj = self->onCreate (self->ptr, type);
  }
  
  eclist_push_back (self->stack, item);
  
  self->keyElement = item;
}

//-----------------------------------------------------------------------------

void ecjsonparser_pop (EcJsonParser self)
{
  // transfer all element values to the content element
  {
    EcJsonParserItem* element = eclist_pop_back (self->stack);
    
    ecjsonparser_item_setObject (self->valElement, &(element->obj), element->type);
    
    ecjsonparser_create_stack_onDestroy (element);
  }
  
  // fetch last element
  {
    EcListCursor cursor;
    eclist_cursor_init (self->stack, &cursor, LIST_DIR_PREV);
    
    if (eclist_cursor_next (&cursor))
    {
      self->keyElement = eclist_data(cursor.node);
    }
    else
    {
      self->keyElement = NULL;
    }
  }
}

//-----------------------------------------------------------------------------

int ecjsonparser_leave_value (EcJsonParser self, int state)
{
  if (state == JPARSER_STATE_LIST_RUN)
  {
    ecstream_clear (self->valElement->stream);
    return JPARSER_STATE_VAL_BEG;
  }
  else if (state == JPARSER_STATE_NODE_RUN)
  {
    return JPARSER_STATE_KEY_BEG;
  }
  
  return JPARSER_STATE_NONE;
}

//-----------------------------------------------------------------------------

void ecjsonparser_decode_unicode (unsigned int unicode, EcStream dest)
{
  if (unicode < 0x80)
  {
    ecstream_append_c (dest, unicode);
  }
  else if (unicode < 0x800)
  {
    ecstream_append_c (dest, 192 + unicode / 64);
    ecstream_append_c (dest, 128 + unicode % 64);
  }
  else if (unicode - 0xd800u < 0x800)
  {
    // error
  }
  else if (unicode < 0x10000)
  {
    ecstream_append_c (dest, 224 + unicode / 4096);
    ecstream_append_c (dest, 128 + unicode /64 % 64);
    ecstream_append_c (dest, 128 + unicode % 64);
  }
  else if (unicode < 0x110000)
  {
    ecstream_append_c (dest, 240 + unicode / 262144);
    ecstream_append_c (dest, 128 + unicode / 4096 % 64);
    ecstream_append_c (dest, 128 + unicode / 64 % 64);
    ecstream_append_c (dest, 128 + unicode % 64);
  }
  else
  {
    // error
  }
}

//-----------------------------------------------------------------------------

unsigned int ecjsonparser_decode_unicode_point (const char** pc)
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

//-----------------------------------------------------------------------------

void ecjsonparser_decode_unicode_hex (EcJsonParser self, EcStream dest)
{
  // convert from hex into decimal
  wchar_t wc = strtol ((const char*)self->unicode_data, NULL, 16);
  
  //printf ("HEX: %s -> %u\n", self->unicode_data, wc);
  
  if ( 0 <= wc && wc <= 0x7f )
  {
    ecstream_append_c (dest, wc);
  }
  else if ( 0x80 <= wc && wc <= 0x7ff )
  {
    ecstream_append_c (dest, 0xc0 | (wc >> 6));
    ecstream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x800 <= wc && wc <= 0xffff )
  {
    ecstream_append_c (dest, 0xe0 | (wc >> 12));
    ecstream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f));
    ecstream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x10000 <= wc && wc <= 0x1fffff )
  {
    ecstream_append_c (dest, 0xf0 | (wc >> 18));
    ecstream_append_c (dest, 0x80 | ((wc >> 12) & 0x3f));
    ecstream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f));
    ecstream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x200000 <= wc && wc <= 0x3ffffff )
  {
    ecstream_append_c (dest, 0xf8 | (wc >> 24));
    ecstream_append_c (dest, 0x80 | ((wc >> 18) & 0x3f));
    ecstream_append_c (dest, 0x80 | ((wc >> 12) & 0x3f));
    ecstream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f));
    ecstream_append_c (dest, 0x80 | (wc & 0x3f));
  }
  else if ( 0x4000000 <= wc && wc <= 0x7fffffff )
  {
    ecstream_append_c (dest, 0xfc | (wc >> 30) );
    ecstream_append_c (dest, 0x80 | ((wc >> 24) & 0x3f) );
    ecstream_append_c (dest, 0x80 | ((wc >> 18) & 0x3f) );
    ecstream_append_c (dest, 0x80 | ((wc >> 12) & 0x3f) );
    ecstream_append_c (dest, 0x80 | ((wc >> 6) & 0x3f) );
    ecstream_append_c (dest, 0x80 | (wc & 0x3f) );
  }
}

//-----------------------------------------------------------------------------

void ecjsonparser_item_next (EcJsonParser self, int type, const char* key, int index)
{
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_TEXT:
    {
      if (self->onItem)
      {
        // void* ptr, void* obj, int type, const char* key, void* val
        self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_TEXT, (void*)ecstream_get (self->valElement->stream), key, index);
      }
      
      ecstream_clear (self->valElement->stream);
      break;
    }
    case ENTC_JPARSER_OBJECT_NUMBER:
    {
      if (self->onItem)
      {
        char* endptr = NULL;
        const char* val = ecstream_get (self->valElement->stream);

#ifdef _WIN32
		int64_t dat = atoi (val);
#else
        int64_t dat = strtoll (val, &endptr, 10);
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
      
      ecstream_clear (self->valElement->stream);
      break;
    }
    case ENTC_JPARSER_OBJECT_FLOAT:
    {      
      if (self->onItem)
      {
        char* endptr = NULL;
        const char* val = ecstream_get (self->valElement->stream);
        
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
      
      ecstream_clear (self->valElement->stream);
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
          const char* val = ecstream_get (self->valElement->stream);
          
          // we need to detect the kind of element
          switch (val[0])
          {
            case 't':
            {
              if (ecstr_equal("true", val) && self->onItem)
              {
                self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_BOLEAN, (void*)TRUE, key, index);
              }
              
              ecstream_clear (self->valElement->stream);

              break;
            }
            case 'f':
            {
              if (ecstr_equal("false", val) && self->onItem)
              {
                self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_BOLEAN, (void*)FALSE, key, index);
              }
              
              ecstream_clear (self->valElement->stream);

              break;
            }
            case 'n':
            {
              if (ecstr_equal("null", val) && self->onItem)
              {
                self->onItem (self->ptr, self->keyElement->obj, ENTC_JPARSER_OBJECT_NULL, NULL, key, index);
              }
              
              ecstream_clear (self->valElement->stream);

              break;
            }
          }
          
          ecstream_clear (self->valElement->stream);
          
          break;
        }
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------

int ecjsonparser_item (EcJsonParser self, int type)
{
  if (self->keyElement)
  {
    switch (self->keyElement->state)
    {
      case JPARSER_STATE_NODE_RUN:
      {
        ecjsonparser_item_next (self, type, ecstream_get (self->keyElement->stream), 0);
        
        ecstream_clear (self->keyElement->stream);
        break;
      }
      case JPARSER_STATE_LIST_RUN:
      {
        ecjsonparser_item_next (self, type, NULL, self->keyElement->index);
        
        self->keyElement->index++;
        break;
      }
    }
    
    return self->keyElement->state;
  }
  
  return JPARSER_STATE_NONE;
}

//-----------------------------------------------------------------------------

int ecjsonparser_parse (EcJsonParser self, const char* buffer, int len, EcErr err)
{
  const char* c = buffer;
  int i;
  
  int state = self->valElement->state;
  
  for (i = 0; (i < len) && *c; i++, c++)
  {
    switch (*c)
    {
      case '{':
      {
        switch (state)
        {
          case JPARSER_STATE_NONE:
          {
            ecjsonparser_push (self, ENTC_JPARSER_OBJECT_NODE, JPARSER_STATE_NODE_RUN);
            
            state = JPARSER_STATE_KEY_BEG;
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            ecjsonparser_push (self, ENTC_JPARSER_OBJECT_NODE, JPARSER_STATE_NODE_RUN);
            
            state = JPARSER_STATE_KEY_BEG;
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in '{'");
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
            ecstream_clear (self->valElement->stream);
            
            ecjsonparser_push (self, ENTC_JPARSER_OBJECT_LIST, JPARSER_STATE_LIST_RUN);
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = JPARSER_STATE_VAL_BEG;
            ecstream_clear (self->valElement->stream);
            
            ecjsonparser_push (self, ENTC_JPARSER_OBJECT_LIST, JPARSER_STATE_LIST_RUN);
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in '['");
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
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_KEY_BEG:
          {
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_UNDEFINED);
            
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_NUMBER);
            
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_FLOAT);
            
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in '}'");
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
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_UNDEFINED);
            
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_NUMBER);
            
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_FLOAT);
            
            ecjsonparser_pop (self);
            
            if (self->keyElement)
            {
              state = JPARSER_STATE_VAL_BEG;
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in ']'");
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
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_TEXT);
            break;
          }
          case JPARSER_STATE_KEY_ESCAPE:
          {
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            state = JPARSER_STATE_KEY_RUN;
            break;
          }
          case JPARSER_STATE_STR_ESCAPE:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            state = JPARSER_STATE_STR_RUN;
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in '\"'");
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
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            state = JPARSER_STATE_KEY_RUN;
            break;
          }
          case JPARSER_STATE_STR_ESCAPE:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            state = JPARSER_STATE_STR_RUN;
            break;
          }
          default:
          {
            return ecerr_set_fmt (err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state [%i] in '\\'", (int)state);
          
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
            ecstream_clear (self->valElement->stream);
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_UNDEFINED);
            
            state = ecjsonparser_leave_value (self, state);
            
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_NUMBER);
            
            state = ecjsonparser_leave_value (self, state);
            
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            state = ecjsonparser_item (self, ENTC_JPARSER_OBJECT_FLOAT);
            
            state = ecjsonparser_leave_value (self, state);
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_KEY_RUN:    // end of key
          {
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in ','");
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
            ecstream_clear (self->valElement->stream);
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in ':'");
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
            ecstream_append_c (self->valElement->stream, *c);
            
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
            
            ecstream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_NUMBER_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_FLOAT_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_KEY_RUN:
          {
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_KEY_UNICODE:
          {
            self->unicode_data [self->unicode_pos + 2] = *c;
            self->unicode_pos++;
            
            if (self->unicode_pos == 4)
            {
              EcJsonParserItem* element = self->keyElement;
              
              if (element)
              {
                ecjsonparser_decode_unicode_hex (self, element->stream);
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
              ecjsonparser_decode_unicode_hex (self, self->valElement->stream);              
              state = JPARSER_STATE_STR_RUN;
            }
           
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state by number");
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
            
            ecstream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_KEY_RUN:
          {
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in '-'");
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
            
            ecstream_append_c (self->valElement->stream, *c);
            break;
          }
          case JPARSER_STATE_KEY_RUN:
          {
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state in '.'");
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
            EcJsonParserItem* element = self->keyElement;
            
            if (element)
            {
              ecstream_append_c (element->stream, *c);
            }
            
            break;
          }
          case JPARSER_STATE_STR_RUN:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          case JPARSER_STATE_KEY_ESCAPE:
          {
            EcJsonParserItem* element = self->keyElement;
            
            if (element) switch (*c)
            {
              case '/':
              {
                ecstream_append_c (element->stream, '/');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'n':
              {
                ecstream_append_c (element->stream, '\n');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 't':
              {
                ecstream_append_c (element->stream, '\t');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'r':
              {
                ecstream_append_c (element->stream, '\r');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'b':
              {
                ecstream_append_c (element->stream, '\b');
                
                state = JPARSER_STATE_KEY_RUN;
                break;
              }
              case 'f':
              {
                ecstream_append_c (element->stream, '\f');
                
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
                eclog_fmt (LL_WARN, "ENTC", "ecjparser", "unknown escape sequence '%c' -> [%i]", *c, *c);
                
                ecstream_append_c (element->stream, *c);
                
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
                ecstream_append_c (self->valElement->stream, '/');

                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'n':
              {
                ecstream_append_c (self->valElement->stream, '\n');

                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 't':
              {
                ecstream_append_c (self->valElement->stream, '\t');

                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'r':
              {
                ecstream_append_c (self->valElement->stream, '\r');

                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'b':
              {
                ecstream_append_c (self->valElement->stream, '\b');

                state = JPARSER_STATE_STR_RUN;
                break;
              }
              case 'f':
              {
                ecstream_append_c (self->valElement->stream, '\f');

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
                eclog_fmt (LL_WARN, "ENTC", "ecjparser", "unknown escape sequence '%c' -> [%i]", *c, *c);
                
                ecstream_append_c (self->valElement->stream, *c);

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
              EcJsonParserItem* element = self->keyElement;
              
              if (element)
              {
                ecjsonparser_decode_unicode_hex (self, element->stream);
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
              ecjsonparser_decode_unicode_hex (self, self->valElement->stream);
              state = JPARSER_STATE_STR_RUN;
            }
           
            break;
          }
          case JPARSER_STATE_VAL_BEG:
          {
            ecstream_append_c (self->valElement->stream, *c);
            
            break;
          }
          default:
          {
            return ecerr_set_fmt (err, ENTC_LVL_ERROR, ENTC_ERR_PARSER, "unexpected state [%i] in default '%c' <- [%i]", state, *c, *c);
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

void* ecjsonparser_lastObject (EcJsonParser self)
{
  void* obj = self->valElement->obj;
  
  self->valElement->obj = NULL;
  
  return obj;
}

//-----------------------------------------------------------------------------
