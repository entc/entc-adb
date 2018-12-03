#include "ectemplate.h"

#include <fcntl.h>

// entc includes
#include "types/ecstream.h"
#include "tools/eclog.h"

// q6 includes
#include "types/ecmap.h"

#define PART_TYPE_NONE   0
#define PART_TYPE_TEXT   1
#define PART_TYPE_FILE   2
#define PART_TYPE_TAG    3
#define PART_TYPE_NODE   4
#define PART_TYPE_CR     5

//-----------------------------------------------------------------------------

struct EcTemplatePart_s; typedef struct EcTemplatePart_s* EcTemplatePart;

struct EcTemplatePart_s
{

   int type;

   EcString text;
   
   EcList parts;
   
   EcTemplatePart parent;

}; 

//-----------------------------------------------------------------------------

EcTemplatePart ectemplate_part_new (int type, const EcString text, EcTemplatePart parent)
{
  EcTemplatePart self = ENTC_NEW (struct EcTemplatePart_s);
  
  self->type = type;
  self->text = ecstr_copy (text);
  
  self->parts = NULL;
  
  self->parent = parent;

  return self;
}

//-----------------------------------------------------------------------------

void ectemplate_part_del (EcTemplatePart* p_self)
{
  EcTemplatePart self = *p_self;
  
  ecstr_delete(&(self->text));
  
  if (self->parts)
  {
    eclist_destroy (&(self->parts));
  }
  
  ENTC_DEL (p_self, struct EcTemplatePart_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL ectemplate_create_parts_onDestroy (void* ptr)
{
  EcTemplatePart h = ptr; ectemplate_part_del (&h);  
  return 0;
}

//-----------------------------------------------------------------------------

int ectemplate_part_hasText (EcTemplatePart self, const EcString text)
{
  return ecstr_equal (self->text, text);
}

//-----------------------------------------------------------------------------

EcTemplatePart ectemplate_part_parent (EcTemplatePart self)
{
  return self->parent;
}

//-----------------------------------------------------------------------------

void ectemplate_part_add (EcTemplatePart self, EcTemplatePart part)
{
  if (self->parts == NULL)
  {
    self->parts = eclist_create (ectemplate_create_parts_onDestroy);
  }

  eclist_push_back (self->parts, part);
}  
  
//-----------------------------------------------------------------------------

int ectemplate_part_apply (EcTemplatePart self, EcUdc data, void* ptr, fct_ectemplate_onText onText, fct_ectemplate_onFile onFile, EcErr err)
{
  EcListCursor cursor; eclist_cursor_init (self->parts, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    EcTemplatePart part = eclist_data (cursor.node);
    
    switch (part->type)
    {
      case PART_TYPE_TEXT:
      case PART_TYPE_CR:
      {
        if (onText)
        {
          onText (ptr, part->text);
        }
      }
      break;
      case PART_TYPE_FILE:
      {
        if (onFile)
        {
          onFile (ptr, part->text);
        }
      }
      break;
      case PART_TYPE_TAG:
      {
        const EcString name = part->text;
        
        EcUdc item = ecudc_node(data, name);
        if (item)
        {
          switch (ecudc_type(item))
          {
            case ENTC_UDC_LIST:
            {
              void* cursor = NULL;
              EcUdc item_list;
              
              for (item_list = ecudc_next(item, &cursor); item_list; item_list = ecudc_next(item, &cursor))
              {
                int res = ectemplate_part_apply (part, item_list, ptr, onText, onFile, err);
                if (res)
                {
                  return res;
                }
                
              }
              
              break;
            }
            case ENTC_UDC_NODE:
            {
              
             
              break;
            }
            case ENTC_UDC_STRING:
            {
              if (onText)
              {
                onText (ptr, ecudc_asString(item));
              }
              
              break;
            }
          }
        }
      }
      break;
    }
  }
  
  return ENTC_ERR_NONE;
}
  
  
//-----------------------------------------------------------------------------

struct EcTemplate_s
{

   EcString fileName;

   EcTemplatePart root_part;

};

//-----------------------------------------------------------------------------

struct EcTemplateCompiler_s
{

  int state;

  EcStream sb;
  
  EcTemplatePart part;   // reference

}; typedef struct EcTemplateCompiler_s* EcTemplateCompiler;

//-----------------------------------------------------------------------------

EcTemplateCompiler entc_template_compiler_new (EcTemplatePart part)
{
  EcTemplateCompiler self = ENTC_NEW (struct EcTemplateCompiler_s);
  
  self->state = 0;
  self->sb = ecstream_create ();
  
  self->part = part;
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_template_compiler_del (EcTemplateCompiler* p_self)
{
  EcTemplateCompiler self = *p_self;
  
  ecstream_destroy (&(self->sb));
  
  ENTC_DEL (p_self, struct EcTemplateCompiler_s);
}

//-----------------------------------------------------------------------------

int entc_template_compiler_part (EcTemplateCompiler self, int type, EcErr err)
{
  switch (type)
  {
    case PART_TYPE_TEXT:
    case PART_TYPE_FILE:
    {
      ectemplate_part_add (self->part, ectemplate_part_new (type, ecstream_get (self->sb), NULL));
      
      ecstream_clear (self->sb);
      
      break;
    }
    case PART_TYPE_TAG:
    {
      const EcString name = ecstream_get (self->sb);
      
      switch (name[0])
      {
        case '#':
        {
          EcTemplatePart new_part = ectemplate_part_new (type, name + 1, self->part);
                    
          // add the new part to the current part
          ectemplate_part_add (self->part, new_part);

          // now change the current part to the new part, that we go one level up
          self->part = new_part;
          
          break;
        }
        case '/':
        {
          // is the current part the ending tag 
          if (ectemplate_part_hasText (self->part, name + 1))
          {
            // has the current part a parent
            EcTemplatePart parent_part = ectemplate_part_parent (self->part);
            if (parent_part)
            {
              // change back the current part to the parent, that we go one level down
              self->part = parent_part;
            }
          }
                    
          break;
        }
        default:
        {
          ectemplate_part_add (self->part, ectemplate_part_new (type, name, self->part));
          
          break;
        }
      }
      
      // always clear
      ecstream_clear (self->sb);
      
      break;
    }
    case PART_TYPE_CR:
    {
      // always clear
      ecstream_clear (self->sb);
      
      break;
    }
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int entc_template_compiler_parse (EcTemplateCompiler self, const char* buffer, int size, EcErr err)
{
  int res;
  
  const char* c = buffer;
  int i;
  
  for (i = 0; i < size; i++, c++)
  {
    switch (self->state)
    {
      case 0:
      {
        if (*c == '{')
        {
          self->state = 1;
        }
        else if (*c == '[')
        {
          self->state = 4;
        }
        else
        {
          ecstream_append_c (self->sb, *c);
        }
      }
      break;
      case 1:
      {
        if (*c == '{')
        {
          res = entc_template_compiler_part (self, PART_TYPE_TEXT, err);
          if (res)
          {
            return res;
          }
          
          self->state = 2;
        }
        else
        {
          self->state = 0;
          ecstream_append_c (self->sb, '{');
          ecstream_append_c (self->sb, *c);
        }
      }
      break;
      case 2:
      {
        if (*c == '}')
        {
          self->state = 3;
        }
        else
        {
          ecstream_append_c (self->sb, *c);
        }
      }
      break;
      case 3:
      {
        if (*c == '}')
        {
          res = entc_template_compiler_part (self, PART_TYPE_TAG, err);
          if (res)
          {
            return res;
          }
          
          self->state = 7;
        }
        else
        {
          self->state = 2;
          ecstream_append_c (self->sb, '}');
        }
      }
      break;
      case 4:
      {
        if (*c == '[')
        {
          res = entc_template_compiler_part (self, PART_TYPE_TEXT, err);
          if (res)
          {
            return res;
          }
          
          self->state = 5;
        }
        else
        {
          self->state = 0;
          ecstream_append_c (self->sb, '[');
          ecstream_append_c (self->sb, *c);
        }
      }
      break;
      case 5:
      {
        if (*c == ']')
        {
          self->state = 6;
        }
        else
        {
          ecstream_append_c (self->sb, *c);
        }
      }
      break;
      case 6:
      {
        if (*c == ']')
        {
          res = entc_template_compiler_part (self, PART_TYPE_FILE, err);
          if (res)
          {
            return res;
          }
          
          self->state = 7;
        }
        else
        {
          self->state = 5;
        }
      }
      break;
      case 7:   // special state
      {
        if ((*c == '\n')||(*c == '\r'))
        {
          ecstream_append_c (self->sb, *c);

          res = entc_template_compiler_part (self, PART_TYPE_CR, err);
          if (res)
          {
            return res;
          }
          
          self->state = 0;
        }
        else if (*c == '{')
        {
          self->state = 1;
        }
        else if (*c == '[')
        {
          self->state = 4;
        }        
        else
        {
          ecstream_append_c (self->sb, *c);

          self->state = 0;
        }
      }
      break;
    }
  }
  
  // add last part as text
  return entc_template_compiler_part (self, PART_TYPE_TEXT, err);
}

//-----------------------------------------------------------------------------

int entc_template_read (EcTemplateCompiler tcl, EcFileHandle fh, EcBuffer buffer, EcErr err)
{
  int bytesRead;
  
  for (bytesRead = ecfh_readBuffer(fh, buffer); bytesRead > 0; bytesRead = ecfh_readBuffer(fh, buffer))
  {
    int res = entc_template_compiler_parse (tcl, (const char*)buffer->buffer, bytesRead, err);
    if (res)
    {
      return res;
    }
  }

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ectemplate_compile (EcTemplate self, EcErr err)
{
  int res;
  
  EcTemplateCompiler tcl = NULL;
  EcBuffer buffer = NULL;
  
  EcFileHandle fh = ecfh_open(self->fileName, O_RDONLY);
  if (fh == NULL)
  {
    eclog_fmt (LL_ERROR, "Q6_MSGS", "template", "can't open file '%s'", self->fileName);

    res = ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    goto exit_and_cleanup;
  }

  tcl = entc_template_compiler_new (self->root_part);
  
  buffer = ecbuf_create (1024);
    
  res = entc_template_read (tcl, fh, buffer, err);
  
exit_and_cleanup:

  if (buffer)
  {
    ecbuf_destroy (&buffer);
  }

  if (fh)
  {
    ecfh_close(&fh);
  }
  
  if (tcl)
  {
    entc_template_compiler_del (&tcl);
  }

  return res;
}

//-----------------------------------------------------------------------------

int ectemplate_filename (EcTemplate self, const char* path, const char* name, const char* lang, EcErr err)
{
  if (name == NULL)
  {
    return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_WRONG_VALUE, "name is NULL");
  }
  
  if (lang)
  {
    self->fileName = ecstr_catc (lang, '_', name);
  }
  else
  {
    self->fileName = ecstr_copy (name);
  }

  if (path)
  {
    ecstr_replaceTO (&(self->fileName), ecfs_mergeToPath(path, self->fileName));
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

EcTemplate ectemplate_create (void)
{
  int res;
  
  EcTemplate self = ENTC_NEW(struct EcTemplate_s);
  
  self->root_part = ectemplate_part_new (PART_TYPE_TAG, NULL, NULL);
  self->fileName = NULL;

  return self;
}

//-----------------------------------------------------------------------------

void ectemplate_destroy (EcTemplate* p_self)
{
  EcTemplate self = *p_self;
  
  ecstr_delete(&(self->fileName));
  ectemplate_part_del (&(self->root_part));
  
  ENTC_DEL(p_self, struct EcTemplate_s);
}

//-----------------------------------------------------------------------------

int ectemplate_compile_file (EcTemplate self, const char* path, const char* name, const char* lang, EcErr err)
{
  int res;
  
  res = ectemplate_filename (self, path, name, lang, err);
  if (res)
  {
    return res;
  }
  
  return ectemplate_compile (self, err);
}

//-----------------------------------------------------------------------------

int ectemplate_compile_str (EcTemplate self, const char* content, EcErr err)
{
  int res;
  
  EcTemplateCompiler tcl = NULL;
  
  tcl = entc_template_compiler_new (self->root_part);
  
  res = entc_template_compiler_parse (tcl, content, ecstr_len (content), err);
  
exit_and_cleanup:
  
  if (tcl)
  {
    entc_template_compiler_del (&tcl);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

/*
int ectemplate_complete (EcList parts, EcUdc node, void* ptr, fct_ectemplate_onText onText, fct_ectemplate_onFile onFile, EcErr err);

//-----------------------------------------------------------------------------

int ectemplate_node (EcList parts, EcUdc node, void* ptr, fct_ectemplate_onText onText, fct_ectemplate_onFile onFile, EcErr err)
{
  // check different types
  switch (ecudc_type(node))
  {
    case ENTC_UDC_NODE:
    {
      int res = ectemplate_complete (parts, node, ptr, onText, onFile, err);
      if (res)
      {
        return res;
      }
    }
    break;
    case ENTC_UDC_LIST:
    {
      EcUdc item;
      void* cursor;
      
      for (item = ecudc_next(node, &cursor); item; item = ecudc_next(node, &cursor))
      {
        int res = ectemplate_node (parts, item, ptr, onText, onFile, err);
        if (res)
        {
          return res;
        }
      }
    }
    break;
  }
  
  return ENTC_ERR_NONE;
}
*/
//-----------------------------------------------------------------------------

int ectemplate_apply (EcTemplate self, EcUdc node, void* ptr, fct_ectemplate_onText onText, fct_ectemplate_onFile onFile, EcErr err)
{
  return ectemplate_part_apply (self->root_part, node, ptr, onText, onFile, err);
}

//-----------------------------------------------------------------------------

