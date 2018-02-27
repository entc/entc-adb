#include "ecaio_notify.h"

#include "system/macros.h"

#if defined __MS_IOCP

//*****************************************************************************

#include <windows.h>
#include <stdio.h>

#define __NOTIFY_BUF_LEN 1024

struct EcAioNotify_s
{
  HANDLE handle;
  
  FILE_NOTIFY_INFORMATION buffer[__NOTIFY_BUF_LEN];
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioNotify ecaio_notify_create (const char* path)
{
  EcAioNotify self = ENTC_NEW (struct EcAioNotify_s);
  
  self->handle = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_notify_destroy (EcAioNotify* pself)
{
  EcAioNotify self = *pself;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct EcAioNotify_s);
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_notify_fct_destroy (void* ptr)
{
  EcAioNotify self = ptr;
  
  ecaio_notify_destroy (&self);
}

//-----------------------------------------------------------------------------

int ecaio_notify_init (EcAioNotify self, const char* path, EcErr err)
{
  // create the directory if not exists
  if (!CreateDirectory (path, NULL))
  {
    DWORD errCode = GetLastError ();
    if (errCode != ERROR_ALREADY_EXISTS)
    {
      return ecerr_formatErrorOS (err, ENTC_LVL_ERROR, errCode);
    }
  }
  
  // get a handle to the directory
  self->handle = CreateFile (path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
  if (self->handle == INVALID_HANDLE_VALUE)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_notify_read (EcAioNotify self, EcAioContext ctx)
{
  DWORD bytesRead;
  if (!ReadDirectoryChangesW (self->handle, self->buffer, __NOTIFY_BUF_LEN, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY, &bytesRead, (LPOVERLAPPED)ecaio_context_getOverlapped (ctx), NULL))
  {
    EcErr err = ecerr_create ();
    
    ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    
    printf ("filewriter error -> %s\r\n", err->text);
    
    ecerr_destroy (&err);
    
    return ENTC_AIO_CODE_DONE;
  }
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static EcAioStatus __STDCALL ecaio_notify_fct_process (void* ptr, EcAioContext ctx, unsigned long len, unsigned long opt)
{
  EcAioNotify self = ptr;
  
  // first entry
  FILE_NOTIFY_INFORMATION* fni = &(self->buffer[0]);
  
  while (TRUE)
  {
    unsigned long nOffset;
    
    if (self->onNotify && (fni->Action > 0))
    {
      // transform to UTF8
      //char* uname = (char*)malloc(fni->FileNameLength);
      //size_t sname = wcstombs (uname, fni->FileName, fni->FileNameLength);
      
      // translate action from windows
      int action = ENTC_AIO_NOTIFY_NONE;
      
      action |= (fni->Action & FILE_NOTIFY_CHANGE_FILE_NAME) ? ENTC_AIO_NOTIFY_FILE : ENTC_AIO_NOTIFY_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_DIR_NAME) ? ENTC_AIO_NOTIFY_DIR : ENTC_AIO_NOTIFY_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_ATTRIBUTES) ? ENTC_AIO_NOTIFY_ATTR : ENTC_AIO_NOTIFY_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_SIZE) ? ENTC_AIO_NOTIFY_SIZE : ENTC_AIO_NOTIFY_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_LAST_WRITE) ? ENTC_AIO_NOTIFY_WRITE : ENTC_AIO_NOTIFY_NONE;
      
      self->onNotify (self->ptr, action);
      
      //free (uname);
    }
    
    if (fni->NextEntryOffset)
    {
      char *p = (char*)fni;
      fni = (FILE_NOTIFY_INFORMATION*)(p + fni->NextEntryOffset);
    }
    else
    {
      break;
    }
  }
  
  return ecaio_notify_read (self, ctx);
}

//-----------------------------------------------------------------------------

int ecaio_notify_assign (EcAioNotify* pself, EcAio aio, EcErr err)
{
  EcAioNotify self = *pself;
  
  int res = ecaio_append (aio, self->handle, NULL, err);
  if (res)
  {
    ecaio_notify_destroy (&self);
    return res;
  }
  
  {
    // create a async context
    EcAioContext ctx = ecaio_context_create ();
    
    // override callbacks
    ecaio_context_setCallbacks (ctx, self, ecaio_notify_fct_process, ecaio_notify_fct_destroy);
    
    // assign this and the context to the async system
    ecaio_notify_read (self, ctx);
  }
  
  return ENTC_ERR_NONE;
}

//*****************************************************************************

#else

//*****************************************************************************

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#if defined __BSD_KEVENT

//-----------------------------------------------------------------------------

#include <sys/event.h>

//-----------------------------------------------------------------------------

struct EcAioNotify_s
{
  long fd;
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

int ecaio_notify_init (EcAioNotify self, const char* path, EcErr err)
{
#ifdef O_EVTONLY
  self->fd = open (path, O_EVTONLY);
  if (self->fd < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }

  return ENTC_ERR_NONE;
#else

  return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_OS_ERROR, "not supported on this system");
#endif
}

//-----------------------------------------------------------------------------

static EcAioStatus __STDCALL ecaio_notify_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  EcAioNotify self = ptr;
  
  if (self->onNotify)
  {
    int action = ENTC_AIO_NOTIFY_NONE;
    
    action |= (val1 & NOTE_DELETE) ? ENTC_AIO_NOTIFY_DELETE : ENTC_AIO_NOTIFY_NONE;
    action |= (val1 & NOTE_WRITE) ? ENTC_AIO_NOTIFY_WRITE : ENTC_AIO_NOTIFY_NONE;
    action |= (val1 & NOTE_EXTEND) ? ENTC_AIO_NOTIFY_DIR : ENTC_AIO_NOTIFY_NONE;
    action |= (val1 & NOTE_ATTRIB) ? ENTC_AIO_NOTIFY_ATTR : ENTC_AIO_NOTIFY_NONE;
    action |= (val1 & NOTE_RENAME) ? ENTC_AIO_NOTIFY_FILE : ENTC_AIO_NOTIFY_NONE;
    
    self->onNotify (self->ptr, action);
  }
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

#else

//-----------------------------------------------------------------------------

struct EcAioNotify_s
{
  long fd;
  
  fct_ecaio_context_destroy onDestroy;
  
  fct_ecaio_context_onNotify onNotify;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

int ecaio_notify_init (EcAioNotify self, const char* path, EcErr err)
{
  self->fd = 0;
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

static EcAioStatus __STDCALL ecaio_notify_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------

EcAioNotify ecaio_notify_create (const char* path)
{
  EcAioNotify self = ENTC_NEW (struct EcAioNotify_s);
  
  self->fd = 0;
  self->ptr = NULL;
  self->onDestroy = NULL;
  self->onNotify = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_notify_destroy (EcAioNotify* pself)
{
  EcAioNotify self = *pself;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  if (self->fd)
  {
    close (self->fd);
  }
  
  ENTC_DEL (&self, struct EcAioNotify_s);
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_notify_fct_destroy (void* ptr)
{
  EcAioNotify self = ptr;
  
  ecaio_notify_destroy (&self);
}

//-----------------------------------------------------------------------------

int ecaio_notify_assign (EcAioNotify* pself, EcAio aio, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcAioNotify self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_notify_process, ecaio_notify_fct_destroy);
  
  res = ecaio_appendVNode (aio, self->fd, ctx, err);
  if (res)
  {
    ecaio_notify_destroy (pself);
    return res;
  }
  
  *pself = NULL;
  return res;
}

#endif

//*****************************************************************************

void ecaio_notify_setCallback (EcAioNotify self, void* ptr, fct_ecaio_context_onNotify onNotify, fct_ecaio_context_destroy onDestroy)
{
  self->ptr = ptr;
  self->onDestroy = onDestroy;
  self->onNotify = onNotify;
}

//-----------------------------------------------------------------------------

