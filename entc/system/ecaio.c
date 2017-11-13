#include "ecaio.h"

// entc includes
#include <utils/eclogger.h>

#if defined __APPLE__

#include <stdatomic.h>

#endif

//=============================================================================

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
