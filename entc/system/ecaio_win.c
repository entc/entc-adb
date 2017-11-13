#include "ecaio.h"

#include <stdio.h>

// entc inlcudes
#include "system/macros.h"
#include "types/ecbuffer.h"
#include "system/ecmutex.h"
#include "utils/eclogger.h"

#define READ_MAX_BUFFER 1024

#if defined __MS_IOCP

#include <windows.h>

//=============================================================================

Q6AIOContext q6sys_aio_context_create (void)
{
  Q6AIOContext self = ENTC_NEW(struct Q6AIOContext_s);

  // create ref counted callback object
  self->ref = q6sys_aio_refctx_create ();
  
  // initialize overlapped structure
  self->overlapped = (OVERLAPPED_EX*)malloc(sizeof(OVERLAPPED_EX));
  memset(self->overlapped, 0, sizeof(OVERLAPPED_EX));
  
  self->overlapped->ptr = self;
  self->overlapped->cnt = 1;
  
  return self;
}

//-----------------------------------------------------------------------------

void q6sys_aio_context_setCallbacks (Q6AIOContext self, void* ptr, fct_aio_context_process process, fct_aio_context_destroy destroy)
{
  // override callbacks
  q6sys_aio_refctx_setCallbacks (self->ref, ptr, process, destroy);
}

//=============================================================================


//-----------------------------------------------------------------------------

void q6sys_aio_context_destroy (Q6AIOContext* pself)
{
  Q6AIOContext self = *pself;
  
  free (self->overlapped);
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "destroyed %p", self);
  
  q6sys_aio_refctx_decrease (&(self->ref));

  ENTC_DEL(pself, struct Q6AIOContext_s);
}

//-----------------------------------------------------------------------------

int q6sys_aio_context_process (Q6AIOContext self, unsigned long val1, unsigned long val2)
{
  int res = q6sys_aio_refctx_process (self->ref, self, val1, val2);
  
  if (res == OVL_PROCESS_CODE_DONE || res == OVL_PROCESS_CODE_ABORTALL)
  {
    q6sys_aio_context_destroy (&self);
  }
  
  return res;
}

//=============================================================================

struct Q6AIO_s
{

  HANDLE port;

  HANDLE sema;

  EcMutex mutex;

};

//-----------------------------------------------------------------------------

Q6AIO q6sys_aio_create ()
{
  Q6AIO self = ENTC_NEW(struct Q6AIO_s);
  
  self->port = INVALID_HANDLE_VALUE;
  self->sema = INVALID_HANDLE_VALUE;

  self->mutex = ecmutex_new ();
  
  return self;
}

//-----------------------------------------------------------------------------

void q6sys_aio_destroy (Q6AIO* pself)
{
  Q6AIO self = *pself;

  CloseHandle (self->sema);
  CloseHandle (self->port);

  ecmutex_delete (&(self->mutex));

  ENTC_DEL(pself, struct Q6AIO_s);
}

//-----------------------------------------------------------------------------

int q6sys_aio_init (Q6AIO self, Q6Err err)
{
   // initialize the semaphore
   self->sema = CreateSemaphore (NULL, 0, 10000, NULL);
   if (self->sema == INVALID_HANDLE_VALUE)
   {
      return q6err_lastErrorOS (err, Q6LVL_ERROR);
   }

   // initialize windows io completion port
   self->port = CreateIoCompletionPort (INVALID_HANDLE_VALUE, NULL, 0, 0);
   if (self->port  == NULL)
   {
      return q6err_lastErrorOS (err, Q6LVL_ERROR);
   }

   return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

int q6sys_aio_append (Q6AIO self, void* handle, Q6AIOContext ctx, Q6Err err)
{
   // add the handle to the overlapping completion port
   HANDLE cportHandle = CreateIoCompletionPort (handle, self->port, 0, 0);

   // cportHandle must return a value
   if (cportHandle == NULL)
   {
      return q6err_lastErrorOS (err, Q6LVL_ERROR);
   }

   //printf ("new handle was registered %p\r\n", handle);

   return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

int q6sys_aio_addQueueEvent (Q6AIO self, void* ptr, fct_aio_context_process process, fct_aio_context_destroy destroy, Q6Err err)
{
  DWORD t = 0;
  ULONG_PTR ptr2 = (ULONG_PTR)NULL;

  // create a new aio context
  Q6AIOContext ctx = q6sys_aio_context_create ();

  q6sys_aio_context_setCallbacks (ctx, ptr, process, destroy);

  // add the context to the completion port
  PostQueuedCompletionStatus (self->port, t, ptr2, (LPOVERLAPPED)ctx->overlapped);

  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

int q6sys_aio_continue (Q6AIO self, OVERLAPPED_EX* ovl, int repeat, unsigned long bytes)
{
  Q6AIOContext ctx = NULL;
  int cont = OVL_PROCESS_CODE_CONTINUE;

  //ecmutex_lock (self->mutex);

  if (ovl)
  {
    // add locking
    ctx = (Q6AIOContext)(ovl->ptr);
    if (ctx)
    {
      cont = q6sys_aio_context_process(ctx, bytes, 0);
    }
  }

  if (repeat && (cont == OVL_PROCESS_CODE_CONTINUE))
  {

  }
  else if (ctx)
  {

  }

  //ecmutex_unlock (self->mutex);

  return (cont == OVL_PROCESS_CODE_ABORTALL);
}

//-----------------------------------------------------------------------------

int q6sys_aio_wait (Q6AIO self, unsigned long timeout, Q6Err err)
{
   DWORD numOfBytes;
   ULONG_PTR ptr;
   OVERLAPPED* ovl;
   BOOL iores;

   //printf ("wait for event on %p\r\n", self->port);

   iores = GetQueuedCompletionStatus (self->port, &numOfBytes, &ptr, &ovl, timeout);
   if (iores)
   {
      if (q6sys_aio_continue (self, (OVERLAPPED_EX*)ovl, TRUE, numOfBytes))
      {
         return q6err_set (err, Q6LVL_EXPECTED, Q6ERR_PROCESS_ABORT, "wait abborted");
      }
      else
      {
         return Q6ERR_NONE;
      }
   }
   else
   {
      if (ovl == NULL)  // timeout
      {
         //onTimeout ();
      }
      else
      {
         DWORD lastError = GetLastError ();

         if (q6sys_aio_continue (self, (OVERLAPPED_EX*)ovl, FALSE, numOfBytes))
         {
            return q6err_set (err, Q6LVL_EXPECTED, Q6ERR_PROCESS_ABORT, "wait abborted");
         }

         switch (lastError)
         {
            case ERROR_HANDLE_EOF:
            {
               return Q6ERR_NONE;
            }
            case 735: // ERROR_ABANDONED_WAIT_0
            {
               return q6err_set (err, Q6LVL_ERROR, Q6ERR_OS_ERROR, "wait abborted");
            }
            case ERROR_OPERATION_ABORTED: // ABORT
            {
               return q6err_set (err, Q6LVL_ERROR, Q6ERR_OS_ERROR, "wait abborted");
            }
            default:
            {
               //onTimeout ();      
            }
         }
      }

      return Q6ERR_NONE;
   }
}

//-----------------------------------------------------------------------------

static Q6AIO g_aio = NULL;

static int q6sys_aio_wait_ctrl_handler (unsigned long ctrlType)
{
  switch( ctrlType )
  {
    case CTRL_C_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_CLOSE_EVENT:
    {
      Q6Err err = q6err_create ();

      int res = q6sys_aio_abort (g_aio, err);

	  q6err_destroy (&err);

	  Sleep (5000);
	}
    break;
  }

  return 0;
}

//-----------------------------------------------------------------------------

int q6sys_aio_wait_abortOnSignal (Q6AIO self, Q6Err err)
{
  int res;
  g_aio = self;

  if( !SetConsoleCtrlHandler ((PHANDLER_ROUTINE)q6sys_aio_wait_ctrl_handler, TRUE ))
  {
    return Q6ERR_OS_ERROR;
  }

  res = Q6ERR_NONE;
  while (res == Q6ERR_NONE)
  {
    res = q6sys_aio_wait (self, Q6_INFINITE, err);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

static int __STDCALL q6sys_aio_abort_fct_process (void* ptr, Q6AIOContext ctx, unsigned long val1, unsigned long val2)
{
  return OVL_PROCESS_CODE_ABORTALL;  // just tell to abort all
}

//-----------------------------------------------------------------------------

int q6sys_aio_abort (Q6AIO self, Q6Err err)
{
  return q6sys_aio_addQueueEvent (self, NULL, q6sys_aio_abort_fct_process, NULL, err);
}

//-----------------------------------------------------------------------------

#endif
