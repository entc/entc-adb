#include "entc_multipart.h"

//-----------------------------------------------------------------------------

#include "system/ecfile.h"

#include "stc/entc_list.h"
#include "types/ecstream.h"

#include "tools/eclog.h"
#include "tools/eccrypt.h"
#include "tools/eccode.h"
#include "tools/ecmime.h"

//-----------------------------------------------------------------------------

typedef EcBuffer    (__STDCALL *fct_entc_section_onNext) (void* ptr, EcErr err);
typedef void        (__STDCALL *fct_entc_section_onDone) (void* ptr);

//=============================================================================

typedef struct
{
  EcFileHandle fh;
  
  EcBase64Encode base64Encode;
  
  EcDecryptAES decryptAes;
  
  // -----
  
  EcBuffer bufraw;

  // -----
  
  EcString boundary;
  
  EcString path;
  
  EcString name;
  
  int fileId;
  
  EcString vsec;
  
  int aes_type;
  
} EntcSectionFile;

//-----------------------------------------------------------------------------

void* entc_section_file_new (const EcString boundary, const EcString path, const EcString name, int fileId, const EcString vsec, unsigned int aes_type)
{
  EntcSectionFile* self = ENTC_NEW (EntcSectionFile);
 
  self->fh = NULL;
  self->base64Encode = NULL;
  self->decryptAes = NULL;
  
  self->boundary = ecstr_copy (boundary);
  
  self->path = ecstr_copy (path);
  self->name = ecstr_copy (name);
  
  self->fileId = fileId;

  self->vsec = ecstr_copy (vsec);
  self->aes_type = aes_type;
  
  self->bufraw = ecbuf_create (1024);
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL entc_section_file_del (void* ptr)
{
  EntcSectionFile* self = ptr;
  
  if (self->fh)
  {
    ecfh_close (&(self->fh));
  }
  
  if (self->base64Encode)
  {
    eccode_base64_encode_destroy (&(self->base64Encode));
  }
  
  if (self->decryptAes)
  {
    ecdecrypt_aes_destroy (&(self->decryptAes));
  }
  
  ecstr_delete (&(self->boundary));
  ecstr_delete (&(self->path));
  ecstr_delete (&(self->name));
  
  ecstr_delete (&(self->vsec));
  
  ecbuf_destroy (&(self->bufraw));

  ENTC_DEL (&self, EntcSectionFile);}

//-----------------------------------------------------------------------------

EcBuffer entc_section_file_init (EntcSectionFile* self, EcErr err)
{
  self->fh = ecfh_open (self->path, O_RDONLY);
  
  if (self->fh == NULL)
  {
    eclog_fmt (LL_WARN, "ENTC", "mime", "can't open file '%s'", self->path);
    
    // in case of error just continue
    return NULL;
  }
  
  // create a base encode engine
  self->base64Encode = eccode_base64_encode_create ();
  
  // create a decrypt engine
  if (self->vsec)
  {
    self->decryptAes = ecdecrypt_aes_create (self->vsec, self->aes_type, 0);
  }

  const EcString mimeType = ecmime_getFromFile (self->name);
  
  EcStream stream = ecstream_create ();
  
  ecstream_append_str (stream, "--");
  ecstream_append_str (stream, self->boundary);
  ecstream_append_str (stream, "\r\n");
  ecstream_append_str (stream, "Content-Type: ");
  ecstream_append_str (stream, mimeType);
  ecstream_append_str (stream, "; name=\"");
  ecstream_append_str (stream, self->name);
  ecstream_append_str (stream, "\"\r\n");
  ecstream_append_str (stream, "Content-Disposition: INLINE; filename=\"");
  ecstream_append_str (stream, self->name);
  ecstream_append_str (stream, "\"\r\n");
  ecstream_append_str (stream, "Content-ID: <");
  ecstream_append_u64 (stream, self->fileId);
  ecstream_append_str (stream, ">");
  ecstream_append_str (stream, "\r\n");
  ecstream_append_str (stream, "Content-Transfer-Encoding: base64");
  ecstream_append_str (stream, "\r\n\r\n");
  
  return ecstream_tobuf (&stream);  
}

//-----------------------------------------------------------------------------

EcBuffer entc_section_file_finalize (EntcSectionFile* self, EcErr err)
{
  EcBuffer bufcon = NULL;    // do not deallocate
  EcBuffer bufres = NULL;    // is the result buffer
  
  if (self->decryptAes)
  {
    bufcon = ecdecrypt_aes_finalize (self->decryptAes, err);
    if (bufcon == NULL)
    {
      
      
    }
  }
  
  if (self->base64Encode)
  {
    uint_t offset = 0;
    
    if (bufcon)
    {
      bufres = ecbuf_create (eccode_base64_encode_size (bufcon->size) * 2);
      
      offset = eccode_base64_encode_update (self->base64Encode, bufres, bufcon, err);
    }
    else
    {
      // assume the size
      bufres = ecbuf_create (eccode_base64_encode_size (1024));
    }
    
    {
      EcBuffer_s h;
      
      h.buffer = bufres->buffer + offset;
      h.size = bufres->size - offset;

      bufres->size = eccode_base64_encode_finalize (self->base64Encode, &h, err);
    }
  }
  
  return bufres;
}

//-----------------------------------------------------------------------------

EcBuffer entc_section_file_done (EntcSectionFile* self, EcErr err)
{
  EcStream stream = ecstream_create ();
  
  // finallize
  {
    EcBuffer h = entc_section_file_finalize (self, err); 
   
    if (h)
    {
      ecstream_append_ecbuf (stream, h);      
    }
    
    ecbuf_destroy (&h);
  }
  
  return ecstream_tobuf (&stream);
}

//-----------------------------------------------------------------------------

static EcBuffer __STDCALL entc_section_file_next (void* ptr, EcErr err)
{
  EntcSectionFile* self = ptr;
  
  EcBuffer bufcon = NULL;    // do not deallocate
  EcBuffer bufres = NULL;    // is the result buffer
  
  if (self->fh == NULL)
  {
    return entc_section_file_init (self, err);
  }
    
  {  
    // read from file
    uint_t readBytes = ecfh_readBuffer (self->fh, self->bufraw);
    
    if (readBytes == 0) // EOF reached
    { 
      EcBuffer bufcont = entc_section_file_done (self, err);
      
      err->code = ENTC_ERR_EOF;
      
      return bufcont;
    }
    
    bufcon = self->bufraw;
    self->bufraw->size = readBytes;
  }
    
  if (self->decryptAes)
  {
    // try to decrypt the buffer
    bufcon = ecdecrypt_aes_update (self->decryptAes, bufcon, err);

    if (bufcon == NULL)
    {
      eclog_fmt (LL_WARN, "ENTC", "mime", "can't decrypt file '%s'", self->path);

      return NULL;
    }
  }
  
  if (self->base64Encode)
  {
    bufres = ecbuf_create (eccode_base64_encode_size (bufcon->size));
    
    bufres->size = eccode_base64_encode_update (self->base64Encode, bufres, bufcon, err);
  }
  
  return bufres;
}

//=============================================================================

typedef struct
{
  
  EcString boundary;
  
  EcString text;
  
  EcString mimeType;
  
} EntcSectionText;

//-----------------------------------------------------------------------------

void* entc_section_text_new (const EcString boundary, const EcString text, const EcString mimeType)
{
  EntcSectionText* self = ENTC_NEW (EntcSectionText);
  
  self->boundary = ecstr_copy(boundary);
  self->text = ecstr_copy (text);
  self->mimeType = ecstr_copy (mimeType);
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL entc_section_text_del (void* ptr)
{
  EntcSectionText* self = ptr;
  
  ecstr_delete (&(self->boundary));
  ecstr_delete (&(self->text));
  ecstr_delete (&(self->mimeType));
  
  ENTC_DEL (&self, EntcSectionText);
}

//-----------------------------------------------------------------------------

static EcBuffer __STDCALL entc_section_text_next (void* ptr, EcErr err)
{
  EntcSectionText* self = ptr;
  
  EcStream stream = ecstream_create ();
  
  // writing the boundary opening
  ecstream_append_str (stream, "--");
  ecstream_append_str (stream, self->boundary);
  ecstream_append_str (stream, "\r\nContent-Type: ");
  
  if (self->mimeType)
  {
    ecstream_append_str (stream, self->mimeType);
  }
  else
  {
    ecstream_append_str (stream, "text/plain; charset=\"UTF-8\"");
  }
  
  ecstream_append_str (stream, "\r\n\r\n");

  // write the content
  ecstream_append_str (stream, self->text);
  ecstream_append_str (stream, "\r\n");
   
  err->code = ENTC_ERR_EOF;
  
  return ecstream_tobuf (&stream);
}

//=============================================================================

typedef struct
{
  
  EcString boundary;
  
  EcString name;
  
  EcString content;
  
} EntcSectionContentDisposition;

//-----------------------------------------------------------------------------

void* entc_section_cdp_new (const EcString boundary, const EcString name, EcString* p_content)
{
  EntcSectionContentDisposition* self = ENTC_NEW (EntcSectionContentDisposition);
  
  self->boundary = ecstr_copy(boundary);
  self->name = ecstr_copy (name);

  self->content = *p_content;
  *p_content = NULL;
    
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL entc_section_cdp_del (void* ptr)
{
  EntcSectionContentDisposition* self = ptr;
  
  ecstr_delete (&(self->boundary));
  ecstr_delete (&(self->content));
  ecstr_delete (&(self->name));
  
  ENTC_DEL (&self, EntcSectionContentDisposition);
}

//-----------------------------------------------------------------------------

static EcBuffer __STDCALL entc_section_cdp_next (void* ptr, EcErr err)
{
  EntcSectionContentDisposition* self = ptr;
  
  EcStream stream = ecstream_create ();
  
  // writing the boundary opening
  ecstream_append_str (stream, "--");
  ecstream_append_str (stream, self->boundary);
  ecstream_append_str (stream, "\r\nContent-Disposition: name=\"");
  ecstream_append_str (stream, self->name);
  ecstream_append_str (stream, "\"\r\n\r\n");
  
  // write the content
  ecstream_append_str (stream, self->content);
  ecstream_append_str (stream, "\r\n");
  
  err->code = ENTC_ERR_EOF;

  return ecstream_tobuf (&stream);  
}

//=============================================================================

typedef struct
{
  
  EcString boundary;
  
  EcString header;
  
} EntcSectionBegin;

//-----------------------------------------------------------------------------

void* entc_section_begin_new (const EcString boundary, const EcString header)
{
  EntcSectionBegin* self = ENTC_NEW (EntcSectionBegin);
  
  self->boundary = ecstr_copy (boundary);
  self->header = ecstr_copy (header);
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL entc_section_begin_del (void* ptr)
{
  EntcSectionBegin* self = ptr;
  
  ecstr_delete (&(self->boundary));
  ecstr_delete (&(self->header));
  
  ENTC_DEL (&self, EntcSectionBegin);
}

//-----------------------------------------------------------------------------

static EcBuffer __STDCALL entc_section_begin_next (void* ptr, EcErr err)
{
  EntcSectionBegin* self = ptr;
  
  EcStream stream = ecstream_create ();

  // start with header
  if (self->header)
  {
    ecstream_append_str (stream, self->header);
  }
  
  ecstream_append_str (stream, "MIME-Version: 1.0\r\n");
  ecstream_append_str (stream, "Content-Type: multipart/mixed; ");
  ecstream_append_str (stream, "boundary=\"");
  ecstream_append_str (stream, self->boundary);
  ecstream_append_str (stream, "\"");
  
  ecstream_append_str (stream, "\r\n\r\n");  
  
  err->code = ENTC_ERR_EOF;
  
  return ecstream_tobuf (&stream);  
}

//=============================================================================

typedef struct
{
  
  EcString boundary;
  
} EntcSectionEnd;

//-----------------------------------------------------------------------------

void* entc_section_end_new (const EcString boundary)
{
  EntcSectionEnd* self = ENTC_NEW (EntcSectionEnd);
  
  self->boundary = ecstr_copy (boundary);
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL entc_section_end_del (void* ptr)
{
  EntcSectionEnd* self = ptr;
  
  ecstr_delete (&(self->boundary));
  
  ENTC_DEL (&self, EntcSectionEnd);
}

//-----------------------------------------------------------------------------

static EcBuffer __STDCALL entc_section_end_next (void* ptr, EcErr err)
{
  EntcSectionEnd* self = ptr;
  
  EcStream stream = ecstream_create ();
  
  // writing the boundary closing
  ecstream_append_str (stream, "--");
  ecstream_append_str (stream, self->boundary);
  ecstream_append_str (stream, "--");
  
  err->code = ENTC_ERR_EOF;
  
  return ecstream_tobuf (&stream);  
}

//=============================================================================

struct EntcMutilpartSection_s
{
  
  void* ptr;
  
  fct_entc_section_onNext onNext;
  
  fct_entc_section_onDone onDone;
  
  // ---------------
  
  uint_t written;
  
  EcBuffer buf;
  
}; typedef struct EntcMutilpartSection_s* EntcMutilpartSection;

//-----------------------------------------------------------------------------

EntcMutilpartSection entc_multipart_section_new (void* ptr, fct_entc_section_onNext onNext, fct_entc_section_onDone onDone)
{
  EntcMutilpartSection self = ENTC_NEW (struct EntcMutilpartSection_s);
  
  self->ptr = ptr;
  self->onNext = onNext;
  self->onDone = onDone;
  
  self->written = 0;
  self->buf = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_multipart_section_del (EntcMutilpartSection* p_self)
{
  EntcMutilpartSection self = *p_self;
  
  if (self->onDone)
  {
    self->onDone (self->ptr);    
  }
  
  ENTC_DEL (p_self, struct EntcMutilpartSection_s);
}

//-----------------------------------------------------------------------------

uint_t entc_multipart_section_next (EntcMutilpartSection self, EcBuffer bufstream, EcErr err)
{
  if (self->buf == NULL)
  {
    if (self->onNext)
    {
      self->buf = self->onNext (self->ptr, err);
      self->written = 0;
    }
  }

  if (self->buf)
  {
    if ((self->buf->size - self->written) > bufstream->size)
    {
      // copy the section buffer into the bufstream
      memcpy (bufstream->buffer, self->buf->buffer + self->written, bufstream->size);

      self->written += bufstream->size;
      
      return bufstream->size;   // maximum possible
    }
    else
    {
      uint_t bytesToCopy = self->buf->size - self->written;
      
      memcpy (bufstream->buffer, self->buf->buffer + self->written, bytesToCopy);
      
      // we are done with the buffer
      ecbuf_destroy (&(self->buf));
      
      return bytesToCopy;
    }
  }
  
  return 0;
}

//=============================================================================

struct EntcMultipart_s
{
  EntcList parts;
  
  EcString boundary;
  
  EntcMutilpartSection section;
  
};

//-----------------------------------------------------------------------------

static void __STDCALL entc_multipart_section_onDel (void* ptr)
{
  EntcMutilpartSection h = ptr; entc_multipart_section_del (&h);
}

//-----------------------------------------------------------------------------

EntcMultipart entc_multipart_new (const EcString boundary, const EcString header)
{
  EntcMultipart self = ENTC_NEW(struct EntcMultipart_s);
  
  self->parts = entc_list_new (entc_multipart_section_onDel);
  
  if (boundary)
  {
    self->boundary = ecstr_copy(boundary);
  }
  else
  {
    EcBuffer buf = ecbuf_create_uuid ();
    
    self->boundary = ecbuf_str(&buf);
  }
    
  self->section = NULL;

  // add begin of multipart
  {
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_begin_new (self->boundary, header), entc_section_begin_next, entc_section_begin_del);
    
    entc_list_push_back (self->parts, section);
  }
  
  // add end of multipart
  {
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_end_new (self->boundary), entc_section_end_next, entc_section_end_del);
    
    entc_list_push_back (self->parts, section);
  }
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_multipart_del (EntcMultipart* p_self)
{
  EntcMultipart self = *p_self;

  entc_list_del (&(self->parts));
  ecstr_delete (&(self->boundary));
    
  ENTC_DEL(p_self, struct EntcMultipart_s);
}

//-----------------------------------------------------------------------------

void entc_multipart_add_text (EntcMultipart self, const EcString text, const EcString mimeType)
{
  EntcMutilpartSection h = entc_list_pop_back (self->parts);
  
  {
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_text_new (self->boundary, text, mimeType), entc_section_text_next, entc_section_text_del);
    
    entc_list_push_back (self->parts, section);
  }
  
  entc_list_push_back (self->parts, h);
}

//-----------------------------------------------------------------------------

void entc_multipart_add_file (EntcMultipart self, const EcString path, const EcString file, int fileId, const EcString vsec, unsigned int aes_type)
{
  EntcMutilpartSection h = entc_list_pop_back (self->parts);
  
  {
    EcString h = ecfs_mergeToPath (path, file);
    
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_file_new (self->boundary, h, file, fileId, vsec, aes_type), entc_section_file_next, entc_section_file_del);
    
    ecstr_delete (&h);
    
    entc_list_push_back (self->parts, section);
  }

  entc_list_push_back (self->parts, h);
}

//-----------------------------------------------------------------------------

void entc_multipart_add_path (EntcMultipart self, const EcString path, const EcString name, int fileId, const EcString vsec, unsigned int aes_type)
{
  EntcMutilpartSection h = entc_list_pop_back (self->parts);

  {
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_file_new (self->boundary, path, name, fileId, vsec, aes_type), entc_section_file_next, entc_section_file_del);
    
    entc_list_push_back (self->parts, section);
  }

  entc_list_push_back (self->parts, h);
}

//-----------------------------------------------------------------------------

void entc_multipart_add_buf_ot (EntcMultipart self, const EcString name, EcBuffer* p_buf)
{
  EntcMutilpartSection h = entc_list_pop_back (self->parts);

  {
    EcString h = ecbuf_str (p_buf);
    
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_cdp_new (self->boundary, name, &h), entc_section_cdp_next, entc_section_cdp_del);
    
    entc_list_push_back (self->parts, section);
  }

  entc_list_push_back (self->parts, h);
}

//-----------------------------------------------------------------------------

void entc_multipart_add_str (EntcMultipart self, const EcString name, const EcString content)
{
  EntcMutilpartSection h = entc_list_pop_back (self->parts);
  
  {
    EcString h = ecstr_copy (content);
    
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_cdp_new (self->boundary, name, &h), entc_section_cdp_next, entc_section_cdp_del);
    
    entc_list_push_back (self->parts, section);
  }

  entc_list_push_back (self->parts, h);
}

//-----------------------------------------------------------------------------

void entc_multipart_add_str_ot (EntcMultipart self, const EcString name, EcString* p_content)
{ 
  EntcMutilpartSection h = entc_list_pop_back (self->parts);

  {
    EntcMutilpartSection section = entc_multipart_section_new (entc_section_cdp_new (self->boundary, name, p_content), entc_section_cdp_next, entc_section_cdp_del);
    
    entc_list_push_back (self->parts, section);
  }

  entc_list_push_back (self->parts, h);
}

//-----------------------------------------------------------------------------

EcString entc_multipart_content_type (EntcMultipart self)
{
  return NULL;
}

//-----------------------------------------------------------------------------

uint_t entc_multipart_next (EntcMultipart self, EcBuffer buf)
{
  uint_t bytesCopied = 0;
  
  while (bytesCopied < buf->size)
  {
    EcBuffer_s h;
    
    h.buffer = buf->buffer + bytesCopied;
    h.size = buf->size - bytesCopied;
    
    if (self->section == NULL)
    {
      self->section = entc_list_pop_front (self->parts);
    }
    
    if (self->section == NULL)
    {
      return bytesCopied;
    }

    {
      EcErr err = ecerr_create();
      
      bytesCopied += entc_multipart_section_next (self->section, &h, err);
      
      if (err->code == ENTC_ERR_EOF)
      {
        entc_multipart_section_del (&(self->section));
      }
      
      ecerr_destroy (&err);
    }
  }

  return bytesCopied;
}

//-----------------------------------------------------------------------------
