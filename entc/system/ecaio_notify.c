#include "ecaio_notify.h"

#if defined __MS_IOCP

//*****************************************************************************

#define __NOTIFY_BUF_LEN 1024

struct Q6AIONotify_s
{
  HANDLE handle;
  
  FILE_NOTIFY_INFORMATION buffer[__NOTIFY_BUF_LEN];
  
  fct_aio_context_destroy onDestroy;
  
  fct_aio_context_onNotify onNotify;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

Q6AIONotify q6sys_aio_notify_create (const char* path)
{
  Q6AIONotify self = ENTC_NEW (struct Q6AIONotify_s);
  
  self->handle = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void q6sys_aio_notify_destroy (Q6AIONotify* pself)
{
  Q6AIONotify self = *pself;
  
  if (self->onDestroy)
  {
    self->onDestroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct Q6AIONotify_s);
}

//-----------------------------------------------------------------------------

static void __STDCALL q6sys_aio_notify_fct_destroy (void* ptr)
{
  Q6AIONotify self = ptr;
  
  q6sys_aio_notify_destroy (&self);
}

//-----------------------------------------------------------------------------

int q6sys_aio_notify_init (Q6AIONotify self, const char* path, Q6Err err)
{
  // create the directory if not exists
  if (!CreateDirectory (path, NULL))
  {
    DWORD errCode = GetLastError ();
    if (errCode != ERROR_ALREADY_EXISTS)
    {
      return q6err_formatErrorOS (err, Q6LVL_ERROR, errCode);
    }
  }
  
  // get a handle to the directory
  self->handle = CreateFile (path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
  if (self->handle == INVALID_HANDLE_VALUE)
  {
    return q6err_lastErrorOS (err, Q6LVL_ERROR);
  }
  
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

int q6sys_aio_notify_read (Q6AIONotify self, Q6AIOContext ctx)
{
  DWORD bytesRead;
  if (!ReadDirectoryChangesW (self->handle, self->buffer, __NOTIFY_BUF_LEN, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY, &bytesRead, (LPOVERLAPPED)ctx->overlapped, NULL))
  {
    Q6Err err = q6err_create ();
    
    q6err_lastErrorOS (err, Q6LVL_ERROR);
    
    printf ("filewriter error -> %s\r\n", err->text);
    
    q6err_destroy (&err);
    
    return OVL_PROCESS_CODE_DONE;
  }
  
  return OVL_PROCESS_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __STDCALL q6sys_aio_notify_fct_process (void* ptr, Q6AIOContext ctx, unsigned long len, unsigned long opt)
{
  Q6AIONotify self = ptr;
  
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
      int action = Q6_NOTIFY_ACTION_NONE;
      
      action |= (fni->Action & FILE_NOTIFY_CHANGE_FILE_NAME) ? Q6_NOTIFY_ACTION_FILE : Q6_NOTIFY_ACTION_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_DIR_NAME) ? Q6_NOTIFY_ACTION_DIR : Q6_NOTIFY_ACTION_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_ATTRIBUTES) ? Q6_NOTIFY_ACTION_ATTR : Q6_NOTIFY_ACTION_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_SIZE) ? Q6_NOTIFY_ACTION_SIZE : Q6_NOTIFY_ACTION_NONE;
      action |= (fni->Action & FILE_NOTIFY_CHANGE_LAST_WRITE) ? Q6_NOTIFY_ACTION_WRITE : Q6_NOTIFY_ACTION_NONE;
      
      self->onNotify (self->ptr, action);
      
      //free (uname);
    }
    
    nOffset = fni->NextEntryOffset;
    if (nOffset == 0)
    {
      break;
    }
    
    fni += nOffset;
  }
  
  return q6sys_aio_notify_read (self, ctx);
}

//-----------------------------------------------------------------------------

int q6sys_aio_notify_assign (Q6AIONotify* pself, Q6AIO aio, Q6Err err)
{
  Q6AIONotify self = *pself;
  
  int res = q6sys_aio_append (aio, self->handle, NULL, err);
  if (res)
  {
    q6sys_aio_notify_destroy (&self);
    return res;
  }
  
  {
    // create a async context
    Q6AIOContext ctx = q6sys_aio_context_create ();
    
    // override callbacks
    q6sys_aio_context_setCallbacks (ctx, self, q6sys_aio_notify_fct_process, q6sys_aio_notify_fct_destroy);
    
    // assign this and the context to the async system
    q6sys_aio_notify_read (self, ctx);
  }
  
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

void q6sys_aio_notify_setCallback(Q6AIONotify self, void* ptr, fct_aio_context_onNotify onNotify, fct_aio_context_destroy onDestroy)
{
  self->ptr = ptr;
  self->onDestroy = onDestroy;
  self->onNotify = onNotify;
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
  self->fd = open (path, O_EVTONLY);
  if (self->fd < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_notify_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
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
  self->fd = NULL;
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_notify_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  return OVL_PROCESS_CODE_CONTINUE;
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

void ecaio_notify_setCallback(EcAioNotify self, void* ptr, fct_ecaio_context_onNotify onNotify, fct_ecaio_context_destroy onDestroy)
{
  self->ptr = ptr;
  self->onDestroy = onDestroy;
  self->onNotify = onNotify;
}

//-----------------------------------------------------------------------------

