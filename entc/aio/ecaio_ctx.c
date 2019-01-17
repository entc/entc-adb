#include "ecaio_ctx.h"

// entc includes
#include "tools/eclog.h"

//*****************************************************************************

#if defined __MS_IOCP

//*****************************************************************************

#include <windows.h>

//-----------------------------------------------------------------------------

typedef struct _OVERLAPPED_EX
{
  ULONG_PTR Internal;
  ULONG_PTR InternalHigh;
  union {
    struct {
      DWORD Offset;
      DWORD OffsetHigh;
    };
    
    PVOID Pointer;
  };
  
  HANDLE  hEvent;
  
  void* ptr;
  
  unsigned int cnt;
  
} OVERLAPPED_EX, *LPOVERLAPPED_EX;

//-----------------------------------------------------------------------------

struct EcAioContext_s
{
  EcAioRefCtx ref;

  OVERLAPPED_EX* overlapped;
};

//-----------------------------------------------------------------------------

EcAioContext ecaio_context_create (void)
{
  EcAioContext self = ENTC_NEW(struct EcAioContext_s);
  
  // create ref counted callback object
  self->ref = ecaio_refctx_create ();
  
  // initialize overlapped structure
  self->overlapped = (OVERLAPPED_EX*)malloc(sizeof(OVERLAPPED_EX));
  memset(self->overlapped, 0, sizeof(OVERLAPPED_EX));
  
  self->overlapped->ptr = self;
  self->overlapped->cnt = 1;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_context_setCallbacks (EcAioContext self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy)
{
  // override callbacks
  ecaio_refctx_setCallbacks (self->ref, ptr, process, destroy);
}

//-----------------------------------------------------------------------------

void ecaio_context_destroy (EcAioContext* pself)
{
  EcAioContext self = *pself;
  
  free (self->overlapped);
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "destroyed %p", self);
  
  ecaio_refctx_decrease (&(self->ref));
  
  ENTC_DEL(pself, struct EcAioContext_s);
}

//-----------------------------------------------------------------------------

void* ecaio_context_getOverlapped (EcAioContext self)
{
  return self->overlapped;
}

//-----------------------------------------------------------------------------

void ecaio_context_appendOverlappedOffset (EcAioContext self, int offset)
{
  self->overlapped->Offset += offset;
}

//-----------------------------------------------------------------------------

int ecaio_context_process (EcAioContext self, unsigned long val1, unsigned long val2)
{
  int res = ecaio_refctx_process (self->ref, self, val1, val2);
  
  if (res == ENTC_AIO_CODE_DONE || res == ENTC_AIO_CODE_ABORTALL || res == ENTC_AIO_CODE_ONCE)
  {
    ecaio_context_destroy (&self);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

int ecaio_context_continue (void* ptr, int repeat, unsigned long bytes)
{
  OVERLAPPED_EX* ovl = ptr;
  EcAioContext ctx = NULL;
  int cont = ENTC_AIO_CODE_CONTINUE;
  
  //entc_mutex_lock (self->mutex);
  
  if (ovl)
  {
    // add locking
    ctx = (EcAioContext)(ovl->ptr);
    if (ctx)
    {
      cont = ecaio_context_process(ctx, bytes, 0);
    }
  }
  
  if (repeat && (cont == ENTC_AIO_CODE_CONTINUE))
  {
    
  }
  else if (ctx)
  {
    
  }
  
  //entc_mutex_unlock (self->mutex);
  
  return (cont == ENTC_AIO_CODE_ABORTALL);
}

//*****************************************************************************

#else

//*****************************************************************************

struct EcAioContext_s
{
  EcAioRefCtx ref;
  
  void* handle;
};

//-----------------------------------------------------------------------------

EcAioContext ecaio_context_create (void)
{
  EcAioContext self = ENTC_NEW(struct EcAioContext_s);
  
  self->handle = NULL;
  self->ref = ecaio_refctx_create ();
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_context_setCallbacks (EcAioContext self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy)
{
  // override callbacks
  ecaio_refctx_setCallbacks (self->ref, ptr, process, destroy);
}

//-----------------------------------------------------------------------------

void ecaio_context_setHandle (EcAioContext self, void* handle)
{
  self->handle = handle;
}

//-----------------------------------------------------------------------------

void* ecaio_context_getHandle (EcAioContext self)
{
  return self->handle;
}

//-----------------------------------------------------------------------------

void ecaio_context_destroy (EcAioContext* pself)
{
  EcAioContext self = *pself;
  
  ecaio_refctx_decrease(&(self->ref));
  
  ENTC_DEL(pself, struct EcAioContext_s);
}

//-----------------------------------------------------------------------------

int ecaio_context_process (EcAioContext self, unsigned long val1, unsigned long val2)
{
  int res = ecaio_refctx_process (self->ref, self, val1, val2);
  
  if (res == ENTC_AIO_CODE_DONE || res == ENTC_AIO_CODE_ABORTALL || res == ENTC_AIO_CODE_ONCE)
  {
    ecaio_context_destroy (&self);
  }
  
  return res;
}

//*****************************************************************************

#endif

//*****************************************************************************

#if defined __APPLE__

#include <stdatomic.h>

#endif

struct EcAioRefCtx_s
{
  
  fct_ecaio_context_process process;
  
  fct_ecaio_context_destroy destroy;
  
  void* ptr;
  
  /*
   #if defined __APPLE__
   
   atomic_int cnt;
   
   #else
   */
  int cnt;
  
  //#endif
  
};

//-----------------------------------------------------------------------------

EcAioRefCtx ecaio_refctx_create ()
{
  EcAioRefCtx self = ENTC_NEW(struct EcAioRefCtx_s);
  
  self->process = NULL;
  self->destroy = NULL;
  self->ptr = NULL;
  
  self->cnt = 1;
  
  return self;
}

//-----------------------------------------------------------------------------

EcAioRefCtx ecaio_refctx_clone (EcAioRefCtx self)
{
  
#if defined __WIN_OS
  InterlockedIncrement (&(self->cnt));
#else
  /*
   #if defined __APPLE__
   
   atomic_fetch_add_explicit (&(self->cnt), 1, memory_order_relaxed);
   
   #else
   */
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
  __sync_add_and_fetch(&(self->cnt), 1);
#else
  (self->cnt)++;
#endif
#endif
  
  //#endif
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "refsock", "clone [%p] %i", self, self->socket);
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_refctx_decrease (EcAioRefCtx* pself)
{
  EcAioRefCtx self = *pself;
  
#if defined __WIN_OS
  int val = InterlockedDecrement (&(self->cnt));
#else
  
  /*
   #if defined __APPLE__
   
   int val = atomic_fetch_sub_explicit (&(self->cnt), 1, memory_order_relaxed);
   
   #else
   */
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
  int val = (__sync_sub_and_fetch(&(self->cnt), 1));
#else
  int val = --(self->cnt);
#endif
#endif
  
  //#endif
  
  if (val == 0)
  {
    //eclogger_fmt (LL_TRACE, "Q6_SOCK", "refctx", "done [%p]", self);
    
    if (self->destroy)
    {
      self->destroy (self->ptr);
    }
    
    ENTC_DEL (pself, struct EcAioRefCtx_s);
  }
  
  *pself = NULL;
}

//-----------------------------------------------------------------------------

void ecaio_refctx_setCallbacks (EcAioRefCtx self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy)
{
  self->ptr = ptr;
  self->process = process;
  self->destroy = destroy;
}

//-----------------------------------------------------------------------------

int ecaio_refctx_process (EcAioRefCtx self, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  int res = ENTC_AIO_CODE_DONE;
  
  if (self->process)
  {
    res = self->process (self->ptr, ctx, val1, val2);
  }
  
  return res;
}

//-----------------------------------------------------------------------------
