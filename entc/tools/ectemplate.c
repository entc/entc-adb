#include "ectemplate.h"

#include <fcntl.h>

// entc includes
#include "types/ecstream.h"
#include "utils/eclogger.h"

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

   EcBuffer text;

   EcList parts;

};

struct EcTemplate_s
{

   EcString fileName;

   EcList parts;

};

struct EcTemplateStateData_s
{

  int state01;

  EcStream sb;

}; typedef struct EcTemplateStateData_s* EcTemplateStateData;

//-----------------------------------------------------------------------------

static int __STDCALL ectemplate_create_parts_onDestroy (void* ptr)
{
  EcTemplatePart part = ptr;
  
  eclist_destroy (&(part->parts));
  ecbuf_destroy(&(part->text));
  
  ENTC_DEL(&part, struct EcTemplatePart_s);
  
  return 0;
}

//-----------------------------------------------------------------------------

void ectemplate_newPart (EcTemplate self, EcTemplateStateData sd, int type)
{
  EcTemplatePart part = ENTC_NEW(struct EcTemplatePart_s);
  
  part->type = type;
  part->text = ecstream_tobuf (&(sd->sb));
  part->parts = eclist_create (ectemplate_create_parts_onDestroy);

  sd->sb = ecstream_create ();
  
  eclist_push_back (self->parts, part);
}

//-----------------------------------------------------------------------------

int ectemplate_parse (EcTemplate self, EcTemplateStateData sd, const char* buffer, int size, EcErr err)
{
  const char* c = buffer;
  int i;
  
  for (i = 0; i < size; i++, c++)
  {
    switch (sd->state01)
    {
      case 0:
      {
        if (*c == '{')
        {
          sd->state01 = 1;
        }
        else if (*c == '[')
        {
          sd->state01 = 4;
        }
        else
        {
          ecstream_append_c (sd->sb, *c);
        }
      }
      break;
      case 1:
      {
        if (*c == '{')
        {
          ectemplate_newPart (self, sd, PART_TYPE_TEXT);
          sd->state01 = 2;
        }
        else
        {
          sd->state01 = 0;
          ecstream_append_c (sd->sb, '{');
        }
      }
      break;
      case 2:
      {
        if (*c == '}')
        {
          sd->state01 = 3;
        }
        else
        {
          ecstream_append_c (sd->sb, *c);
        }
      }
      break;
      case 3:
      {
        if (*c == '}')
        {
          ectemplate_newPart (self, sd, PART_TYPE_TAG);
          sd->state01 = 7;
        }
        else
        {
          sd->state01 = 2;
          ecstream_append_c (sd->sb, '}');
        }
      }
      break;
      case 4:
      {
        if (*c == '[')
        {
          ectemplate_newPart (self, sd, PART_TYPE_TEXT);
          sd->state01 = 5;
        }
        else
        {
          sd->state01 = 0;
          ecstream_append_c (sd->sb, '[');
        }
      }
      break;
      case 5:
      {
        if (*c == ']')
        {
          sd->state01 = 6;
        }
        else
        {
          ecstream_append_c (sd->sb, *c);
        }
      }
      break;
      case 6:
      {
        if (*c == ']')
        {
          ectemplate_newPart (self, sd, PART_TYPE_FILE);
          sd->state01 = 7;
        }
        else
        {
          sd->state01 = 5;
        }
      }
      break;
      case 7:
      {
        if ((*c == '\n')||(*c == '\r'))
        {
          ecstream_append_c (sd->sb, *c);
          ectemplate_newPart (self, sd, PART_TYPE_CR);
        }
        else
        {
          ecstream_append_c (sd->sb, *c);
        }
        
        sd->state01 = 0;
      }
      break;
    }
  }
  
  // add last part as text
  ectemplate_newPart (self, sd, PART_TYPE_TEXT);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void ectemplate_sections_tag (EcTemplate self, EcTemplatePart part, EcMap sections, EcListCursor* cursor)
{
  EcListNode node = cursor->node;
  
  char c = part->text->buffer[0];
  
  EcString name = ecstr_copy ((const char*)part->text->buffer + 1);
  
  switch (c)
  {
    case '#':
    {
      ecmap_append (sections, name, (void*)node);
    }
    break;
    case '/':
    {
      EcMapNode snode = ecmap_find (sections, name);
      if (snode)
      {
        EcListNode iFrom = ecmap_data (snode);
        
        // remove extra chars after node tags
        {
          EcTemplatePart partFrom = eclist_data(iFrom);
          if (partFrom->type == PART_TYPE_CR)
          {
            partFrom->type = PART_TYPE_NONE;
          }
        }
        
        // extract a part
        EcList slice = eclist_slice (self->parts, iFrom, node);
        
        eclist_insert_slice (part->parts, cursor, &slice);
        
        //node = eclist_splice (self->parts, iFrom, node, part->parts);
        
        // override part content
        {
          EcBuffer h = ecbuf_create_str (&name);
        
          ecbuf_destroy(&(part->text));
        
          part->text = h;
        }        

        part->type = PART_TYPE_NODE;
      }
    }
    break;
  }
}

//-----------------------------------------------------------------------------

void __STDCALL q6template_sections_cleanitem (void* p)
{
  EcString s = p;
  
  ecstr_delete(&s);
}

//-----------------------------------------------------------------------------

int ectemplate_sections (EcTemplate self)
{
  EcMap sections = ecmap_create (EC_ALLOC);
  
  EcListCursor cursor;
  
  eclist_cursor_init (self->parts, &cursor, LIST_DIR_NEXT);
  
  while (eclist_cursor_next (&cursor))
  {
    EcTemplatePart part = eclist_data(cursor.node);
    
    switch (part->type)
    {
      case PART_TYPE_TAG:
      {
        ectemplate_sections_tag (self, part, sections, &cursor);
        break;
      }
    }
  }
  
  ecmap_destroy (EC_ALLOC, &sections /*, q6template_sections_cleanitem*/);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int q6template_read (EcTemplate self, EcFileHandle fh, EcBuffer buffer, EcTemplateStateData sd, EcErr err)
{
  int bytesRead;
  
  for (bytesRead = ecfh_readBuffer(fh, buffer); bytesRead > 0; bytesRead = ecfh_readBuffer(fh, buffer))
  {
    int res = ectemplate_parse (self, sd, (const char*)buffer->buffer, bytesRead, err);
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
  
  EcFileHandle fh = ecfh_open(self->fileName, O_RDONLY);
  if (fh == NULL)
  {
    eclogger_fmt (LL_ERROR, "Q6_MSGS", "template", "can't open file '%s'", self->fileName);

    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  {
    EcBuffer buffer;
    struct EcTemplateStateData_s sd;
    sd.state01 = 0;
    sd.sb = ecstream_create ();

    buffer = ecbuf_create (1024);
    
    res = q6template_read (self, fh, buffer, &sd, err);
    
    ecbuf_destroy (&buffer);
    ecstream_destroy (&(sd.sb));
  }
  
  ecfh_close(&fh);
  
  if (res == ENTC_ERR_NONE)
  {
    res = ectemplate_sections (self);
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

EcTemplate ectemplate_create (const char* path, const char* name, const char* lang, EcErr err)
{
  int res;
  
  EcTemplate self = ENTC_NEW(struct EcTemplate_s);
  
  self->parts = eclist_create (ectemplate_create_parts_onDestroy);
  self->fileName = NULL;

  res = ectemplate_filename (self, path, name, lang, err);
  if (res == 0)
  {
    res = ectemplate_compile (self, err);
  }
  
  if (res)
  {
    ectemplate_destroy (&self);
  }
  
  return self;
}

//-----------------------------------------------------------------------------

void ectemplate_destroy (EcTemplate* pself)
{
  EcTemplate self = *pself;
  
  ecstr_delete(&(self->fileName));
  eclist_destroy (&(self->parts));
  
  ENTC_DEL(pself, struct EcTemplate_s);
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

int ectemplate_complete (EcList parts, EcUdc data, void* ptr, fct_ectemplate_onText onText, fct_ectemplate_onFile onFile, EcErr err)
{
  EcListCursor cursor;
  
  eclist_cursor_init (parts, &cursor, LIST_DIR_NEXT);
  
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
          onText (ptr, (const char*)part->text->buffer);
        }
      }
      break;
      case PART_TYPE_FILE:
      {
        if (onFile)
        {
          onFile (ptr, (const char*)part->text->buffer);
        }
      }
      break;
      case PART_TYPE_TAG:
      {
        EcUdc item = ecudc_node(data, (const char*)part->text->buffer);
        if (item)
        {
          if (onText)
          {
            onText (ptr, ecudc_asString(item));
          }
        }
      }
      break;
      case PART_TYPE_NODE:
      {
        EcUdc item = ecudc_node(data, (const char*)part->text->buffer);
        if (item)
        {
          int res = ectemplate_node (part->parts, item, ptr, onText, onFile, err);
          if (res)
          {
            return res;
          }
        }
      }
      break;
    }
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ectemplate_apply (EcTemplate self, EcUdc node, void* ptr, fct_ectemplate_onText onText, fct_ectemplate_onFile onFile, EcErr err)
{
   return ectemplate_complete (self->parts, node, ptr, onText, onFile, err);
}

//-----------------------------------------------------------------------------

