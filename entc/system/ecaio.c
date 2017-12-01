#include "ecaio.h"

#include "utils/eclogger.h"
#include "utils/ecmessages.h"
#include "system/ecmutex.h"

//*****************************************************************************

#if defined __MS_IOCP

//*****************************************************************************

#include <windows.h>

//-----------------------------------------------------------------------------

struct EcAio_s
{

  HANDLE port;

  HANDLE sema;

  EcMutex mutex;

};

//-----------------------------------------------------------------------------

static EcAio g_aio = NULL;
static int g_termOnly = FALSE;

//-----------------------------------------------------------------------------

EcAio ecaio_create ()
{
  EcAio self = ENTC_NEW(struct EcAio_s);
  
  self->port = INVALID_HANDLE_VALUE;
  self->sema = INVALID_HANDLE_VALUE;
  
  self->mutex = ecmutex_new ();
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_destroy (EcAio* pself)
{
  EcAio self = *pself;
  
  CloseHandle (self->sema);
  CloseHandle (self->port);
  
  ecmutex_delete (&(self->mutex));
  
  ENTC_DEL(pself, struct EcAio_s);
}

//-----------------------------------------------------------------------------

int ecaio_init (EcAio self, EcErr err)
{
  // initialize the semaphore
  self->sema = CreateSemaphore (NULL, 0, 10000, NULL);
  if (self->sema == INVALID_HANDLE_VALUE)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  // initialize windows io completion port
  self->port = CreateIoCompletionPort (INVALID_HANDLE_VALUE, NULL, 0, 0);
  if (self->port  == NULL)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_append (EcAio self, void* handle, EcAioContext ctx, EcErr err)
{
  // add the handle to the overlapping completion port
  HANDLE cportHandle = CreateIoCompletionPort (handle, self->port, 0, 0);
  
  // cportHandle must return a value
  if (cportHandle == NULL)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  //printf ("new handle was registered %p\r\n", handle);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_addQueueEvent (EcAio self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy, EcErr err)
{
  DWORD t = 0;
  ULONG_PTR ptr2 = (ULONG_PTR)NULL;
  
  // create a new aio context
  EcAioContext ctx = ecaio_context_create ();
  
  ecaio_context_setCallbacks (ctx, ptr, process, destroy);
  
  // add the context to the completion port
  PostQueuedCompletionStatus (self->port, t, ptr2, (LPOVERLAPPED)ecaio_context_getOverlapped (ctx));
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_appendENode (EcAio self, EcAioContext ctx, void** eh, EcErr err)
{
  *eh = ctx;
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_triggerENode (EcAio self, void* eh, EcErr err)
{
  ecaio_context_process (eh, 0, 0);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_wait (EcAio self, unsigned long timeout, EcErr err)
{
  DWORD numOfBytes;
  ULONG_PTR ptr;
  OVERLAPPED* ovl;
  BOOL iores;
  
  //printf ("wait for event on %p\r\n", self->port);
  
  iores = GetQueuedCompletionStatus (self->port, &numOfBytes, &ptr, &ovl, timeout);
  if (iores)
  {
    if (ecaio_context_continue (ovl, TRUE, numOfBytes))
    {
      return ecerr_set (err, ENTC_LVL_EXPECTED, ENTC_ERR_PROCESS_ABORT, "wait abborted");
    }
    else
    {
      return ENTC_ERR_NONE;
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
      
	  {
        EcErr err = ecerr_create ();

        ecerr_formatErrorOS (err, ENTC_LVL_ERROR, lastError);

		eclogger_fmt (LL_WARN, "ENTC", "wait", "error on io completion port: %s", err->text);

        ecerr_destroy (&err);
	  }

      if (ecaio_context_continue (ovl, FALSE, numOfBytes))
      {        
        return ecerr_set (err, ENTC_LVL_EXPECTED, ENTC_ERR_PROCESS_ABORT, "wait abborted");
      }
      
      switch (lastError)
      {
        case ERROR_HANDLE_EOF:
        {
          return ENTC_ERR_NONE;
        }
        case 735: // ERROR_ABANDONED_WAIT_0
        {
          return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_OS_ERROR, "wait abborted");
        }
        case ERROR_OPERATION_ABORTED: // ABORT
        {
          return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_OS_ERROR, "wait abborted");
        }
        default:
        {
          //onTimeout ();
        }
      }
    }
    
    return ENTC_ERR_NONE;
  }
}

//-----------------------------------------------------------------------------

static int ecaio_wait_ctrl_handler (unsigned long ctrlType)
{
  int abort = FALSE;

  switch( ctrlType )
  {
    case CTRL_C_EVENT:
	{
      if (g_termOnly == FALSE)
	  {
        abort = TRUE;

  	    eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> %s", ctrlType, "ctrl-c");
	  }

	  break;
	}
    case CTRL_SHUTDOWN_EVENT:
	{
      abort = TRUE;

	  eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> %s", ctrlType, "shutdown");
	  break;
	}
    case CTRL_CLOSE_EVENT:
    {
      abort = TRUE;

	  eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> %s", ctrlType, "close");
	  break;
	}
  }
  if (abort)
  {
	  int res;
      EcErr err = ecerr_create ();
      

      res = ecaio_abort (g_aio, err);
      
      ecerr_destroy (&err);
      
      Sleep (5000);
  }

  return 0;
}

//-----------------------------------------------------------------------------

int ecaio_wait_abortOnSignal (EcAio self, int onlyTerm, EcErr err)
{
  int res;
  g_aio = self;
  g_termOnly = onlyTerm;
  
  if( !SetConsoleCtrlHandler ((PHANDLER_ROUTINE)ecaio_wait_ctrl_handler, TRUE ))
  {
    return ENTC_ERR_OS_ERROR;
  }
  
  res = ENTC_ERR_NONE;
  while (res == ENTC_ERR_NONE)
  {
    res = ecaio_wait (self, ENTC_INFINITE, err);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_abort_fct_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  EcAio self = ptr;
  
  // send again
  ecaio_addQueueEvent (self, self, ecaio_abort_fct_process, NULL, NULL);
  
  return ENTC_AIO_CODE_ABORTALL;  // just tell to abort all
}

//-----------------------------------------------------------------------------

int ecaio_abort (EcAio self, EcErr err)
{
  return ecaio_addQueueEvent (self, self, ecaio_abort_fct_process, NULL, err);
}

//*****************************************************************************

#elif defined __LINUX_EPOLL

//*****************************************************************************

#include <sys/eventfd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>

#define Q6_EPOLL_MAXEVENTS 1

//-----------------------------------------------------------------------------

int tfd;

struct EcAio_s
{

  int efd;

  int ufd;

  EcAioContext ctx;

  EcMutex mutex;

  EcMutex iom;

  //int pfd[2];

  EcList ctxs;
  
  EcList events;

  EcMutex eventsm;
  
};

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_events_onDestroy (void* ptr)
{
  EcAioContext ctx = ptr;
  
  ecaio_context_destroy (&ctx);
  
  return 0;
}

//-----------------------------------------------------------------------------

EcAio ecaio_create ()
{
  EcAio self = ENTC_NEW(struct EcAio_s);

  self->efd = 0;
  self->ufd = 0;
  self->ctx = NULL;

  self->ctxs = eclist_create (NULL);

  /*
   self->pfd[0] = NULL;
   self->pfd[1] = NULL;
   */

  self->mutex = ecmutex_new();
  self->iom = ecmutex_new ();

  self->eventsm = ecmutex_new ();
  self->events = eclist_create (ecaio_events_onDestroy);
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_destroy (EcAio* pself)
{
  EcAio self = *pself;
  
  close (self->ufd);
  
  if (self->ctx)
  {
    ecaio_context_destroy (&(self->ctx));
  }
  
  eclist_destroy (&(self->ctxs));
  
  ecmutex_delete(&(self->mutex));
  ecmutex_delete(&(self->iom));
  
  ENTC_DEL(pself, struct EcAio_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_term_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  return ENTC_AIO_CODE_ABORTALL;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_process_signal (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  printf ("received signal [%i]\n", ptr);
  
  
  
  return ENTC_AIO_CODE_ABORTALL;
}

//-----------------------------------------------------------------------------

int ecaio_init (EcAio self, EcErr err)
{
  self->efd = epoll_create1 (0);
  if (self->efd == -1)
  {

  }

  /*
   {
   pipe (self->pfd);
   
   // create a new aio context
   self->ctx = ecaio_context_create ();
   
   ecaio_context_setCallbacks (self->ctx, self, ecaio_event_onEvent, NULL);
   
   ecaio_append (self, self->pfd[0], self->ctx, err);
   }
   */
  
  {
    self->ufd = eventfd (0, 0);
    
    {
      struct epoll_event event;
      
      event.data.ptr = NULL;
      event.events = EPOLLIN | EPOLLET;
      
      int s = epoll_ctl (self->efd, EPOLL_CTL_ADD, self->ufd, &event);
      if (s == -1)
      {
        
      }
    }
    
    //ecaio_append (self, self->ufd, self->ctx, err);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_append (EcAio self, void* handle, EcAioContext ctx, EcErr err)
{
  struct epoll_event event;
  uint64_t hfd = handle;
  
  event.data.ptr = ctx;
  event.events = EPOLLET | EPOLLONESHOT | EPOLLIN;
  
  //eclogger_fmt (LL_TRACE, "ENTC", "context", "append %i", (int)handle, ctx);
  
  int s = epoll_ctl (self->efd, EPOLL_CTL_ADD, hfd, &event);
  if (s < 0)
  {
    int errCode = errno;
    if (errCode == EPERM)
    {
      return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_NOT_SUPPORTED, "file descriptor don't support epoll");
    }
    
    eclogger_fmt (LL_ERROR, "ENTC", "append", "append failed with error %i", s);
    
    return ecerr_formatErrorOS(err, ENTC_LVL_ERROR, errCode);
  }
  
  ecaio_context_setHandle (ctx, handle);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_appendVNode (EcAio self, int fd, void* data, EcErr err)
{
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_appendENode (EcAio self, EcAioContext ctx, void** eh, EcErr err)
{
  ecmutex_lock (self->eventsm);
  
  *eh = eclist_push_back (self->events, ctx);
  
  ecmutex_unlock (self->eventsm);

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_triggerENode (EcAio self, void* eh, EcErr err)
{
  EcAioContext ctx;
  EcListNode node = eh;
  
  ecmutex_lock (self->eventsm);
  
  ctx = eclist_extract (self->events, node);
  
  ecmutex_unlock (self->eventsm);

  return ecaio_addContextToEvent (self, ctx, err);
}

//-----------------------------------------------------------------------------

int ecaio_addContextToEvent (EcAio self, EcAioContext ctx, EcErr err)
{
  ecmutex_lock (self->mutex);
  
  eclist_push_back (self->ctxs, ctx);
  
  ecmutex_unlock (self->mutex);
  
  // trigger event
  uint64_t u = 1;
  //write (self->ufd, &u, sizeof(uint64_t));
  write (self->ufd, &u, sizeof(uint64_t));
  
  //eclogger_fmt (LL_TRACE, "ENTC", "context", "wrote %u", u);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_addQueueEvent (EcAio self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy, EcErr err)
{
  // create a new aio context
  EcAioContext ctx = ecaio_context_create ();
  
  ecaio_context_setCallbacks (ctx, ptr, process, destroy);
  
  //eclogger_fmt (LL_TRACE, "ENTC", "context", "new ctx %p", ctx);
  
  return ecaio_addContextToEvent (self, ctx, err);
}

//-----------------------------------------------------------------------------

void ecaio_abortall (EcAio self)
{
  ecmutex_lock (self->eventsm);
  
  eclist_clear (self->events);
  
  ecmutex_unlock (self->eventsm);
}

//-----------------------------------------------------------------------------

int ecaio_handle_event (EcAio self)
{
  EcAioContext ctx = NULL;

  ecmutex_lock (self->mutex);

  EcListCursor c;
  eclist_cursor_init (self->ctxs, &c, LIST_DIR_NEXT);

  if (eclist_cursor_next (&c))
  {
    ctx = eclist_data(c.node);

    eclist_cursor_erase (self->ctxs, &c);
  }

  ecmutex_unlock (self->mutex);

  if (ctx)
  {
    // trigger next event
    uint64_t u = 1;

    write (self->ufd, &u, sizeof(uint64_t));

    //eclogger_fmt (LL_TRACE, "ENTC", "event", "context %p", ctx);

    int res = ecaio_context_process (ctx, 0, 0);

    if (res == ENTC_AIO_CODE_ABORTALL)
    {
      // send again for other threads
      ecaio_abort (self, NULL);

      ecaio_abortall (self);
      
      // abort
      return ENTC_ERR_NONE_CONTINUE;
    }
  }

  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_wait_signal (EcAio self, unsigned long timeout, sigset_t* sigmask, int onAbort, EcErr err)
{
  int n;
  struct epoll_event *events;

  events = calloc (Q6_EPOLL_MAXEVENTS, sizeof(struct epoll_event));

  //eclogger_fmt (LL_TRACE, "ENTC", "context", "waiting for lock");

  ecmutex_lock (self->iom);

  //eclogger_fmt (LL_TRACE, "ENTC", "context", "waiting for event");

  n = epoll_wait (self->efd, events, Q6_EPOLL_MAXEVENTS, -1);

  eclogger_fmt (LL_TRACE, "ENTC", "context", "got event %i", n);

  if (n < 0)
  {
    ecmutex_unlock (self->iom);

    free (events);

    eclogger_fmt (LL_WARN, "ENTC", "wait", "error on epoll [%i]", onAbort);

    //ecaio_abort (self, NULL);

    //return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);

    return ENTC_ERR_NONE;
  }

  if (n > 0)
  {
    int i = 0;

    EcAioContext ctx;

    ctx = events[i].data.ptr;

    if (ctx)
    {
      // save the handle, because ctx got freed in the process call
      void* handle = ecaio_context_getHandle (ctx);

      //eclogger_fmt (LL_TRACE, "ENTC", "context", "got ctx from IO event %p", ctx);

      int cont = ecaio_context_process (ctx, 0, 0);

      //eclogger_fmt (LL_TRACE, "ENTC", "context", "return %i", cont);

      if (cont == ENTC_AIO_CODE_CONTINUE)
      {
        epoll_ctl (self->efd, EPOLL_CTL_MOD, handle, &(events[i]));
        // continue
      }
      else if (cont == ENTC_AIO_CODE_ABORTALL)
      {
        ecmutex_unlock (self->iom);

	      free (events);
        
        ecaio_abortall (self);

	      ecaio_abort (self, NULL);

        // abort
        return ENTC_ERR_NONE_CONTINUE;
      }
      else
      {
        //eclogger_fmt (LL_TRACE, "ENTC", "context", "remove %i %p", ctx->handle, ctx);
        // remove
        struct epoll_event event = {0}; // see bugs
        int s = epoll_ctl (self->efd, EPOLL_CTL_DEL, handle, &event);
        if (s < 0)
        {


          //eclogger_fmt (LL_ERROR, "ENTC", "wait", "failed to remove file descriptor %i", ctx->handle);
        }
      }

      ecmutex_unlock (self->iom);
    }
    else
    {
      ecmutex_unlock (self->iom);

      int res = ecaio_handle_event (self);

      free (events);

      return res;
    }
  }
  else
  {
    ecmutex_unlock (self->iom);
  }
  
  free (events);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_wait (EcAio self, unsigned long timeout, EcErr err)
{
  sigset_t mask;
  sigset_t orig_mask;
  
  // define the mask
  sigemptyset (&mask);
  
  sigaddset (&mask, SIGINT);
  sigaddset (&mask, SIGTERM);
  
  if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ecaio_wait_signal (self, timeout, &orig_mask, 1, err);
}

//-----------------------------------------------------------------------------

static void ecaio_abort_signalhandler (int sig)
{
  switch (sig)
  {
    case SIGINT:
    {
      eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> %s", sig, "SIGINT");
      break;
    }
    case SIGTERM:
    {
      eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> %s", sig, "SIGTERM");
      break;
    }
  }

  uint64_t u = TRUE;
  write (tfd, &u, sizeof(uint64_t));
}

//-----------------------------------------------------------------------------

static void ecaio_empty_signalhandler (int signum)
{
}

//-----------------------------------------------------------------------------

int ecaio_reset_signals (EcAio self, int onlyTerm, sigset_t* mask, sigset_t* orig_mask, EcErr err)
{
  struct sigaction act;
 
  {
    memset (&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    
    int res = sigaction (SIGINT, &act, NULL);
    if (res)
    {
      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
  }
  {
    memset (&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    
    int res = sigaction (SIGTERM, &act, NULL);
    if (res)
    {
      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
  }
  
  sigemptyset(mask);
  sigaddset(mask, SIGINT);
  sigaddset(mask, SIGQUIT);
  sigaddset(mask, SIGTERM);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_wait_abortOnSignal (EcAio self, int onlyTerm, EcErr err)
{
  int res;
  EcAioContext ctx;
  int sfd;

  sigset_t mask;
  sigset_t orig_mask;

  memset(&mask, 0, sizeof(sigset_t));
  memset(&orig_mask, 0, sizeof(sigset_t));

  res = ecaio_reset_signals (self, onlyTerm, &mask, &orig_mask, err);
  if (res)
  {
    return res;
  }
  
  sfd = signalfd (-1, &mask, 0);
  if (sfd == -1)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  {
    // create a new aio context
    ctx = ecaio_context_create ();
    
    ecaio_context_setCallbacks (ctx, sfd, ecaio_process_signal, NULL);
    
    ecaio_append (self, sfd, ctx, err);
  }
  
  // add terminator
  tfd = eventfd (0, 0);
  if (tfd == -1)
  {

  }

  {
    // create a new aio context
    ctx = ecaio_context_create ();

    ecaio_context_setCallbacks (ctx, NULL, ecaio_term_process, NULL);

    ecaio_append (self, tfd, ctx, err);
  }

  res = ENTC_ERR_NONE;
  while (res == ENTC_ERR_NONE)

  {
    res = ecaio_wait_signal (self, ENTC_INFINITE, &orig_mask, 2, err);
  }

  close (tfd);

  return res;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_abort_fct_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  return ENTC_AIO_CODE_ABORTALL;  // just tell to abort all
}

//-----------------------------------------------------------------------------

int ecaio_abort (EcAio self, EcErr err)
{
  return ecaio_addQueueEvent (self, NULL, ecaio_abort_fct_process, NULL, err);
}

//*****************************************************************************

#elif defined __BSD_KEVENT

//*****************************************************************************

#include <sys/event.h>
#include <errno.h>

//-----------------------------------------------------------------------------

struct EcAio_s
{
  int kq;
};

//-----------------------------------------------------------------------------

EcAio ecaio_create ()
{
  EcAio self = ENTC_NEW(struct EcAio_s);
  
  self->kq = 0;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_destroy (EcAio* pself)
{
  EcAio self = *pself;
  
  close(self->kq);
  
  ENTC_DEL(pself, struct EcAio_s);
}

//-----------------------------------------------------------------------------

int ecaio_init (EcAio self, EcErr err)
{
  self->kq = kqueue ();
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_append (EcAio self, void* handle, EcAioContext ctx, EcErr err)
{
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  int res;
  
  if (ctx == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "add event", "context is NULL");
    
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_WRONG_VALUE, "ctx is null");
  }
  
  EV_SET (&kev, handle, EVFILT_READ | EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, ctx);
  
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_addContextToEvent (EcAio self, EcAioContext ctx, EcErr err)
{
  int res;
  
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  
  if (ctx == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "add event", "context is NULL");
    
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_WRONG_VALUE, "ctx is null");
  }
  
  EV_SET(&kev, ctx, EVFILT_USER, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, ctx);
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  memset (&kev, 0x0, sizeof(struct kevent));
  EV_SET(&kev, ctx, EVFILT_USER, 0, NOTE_TRIGGER, 0, ctx);
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_addQueueEvent (EcAio self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy, EcErr err)
{
  // create a new aio context
  EcAioContext context = ecaio_context_create ();
  if (context == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "add event", "context is NULL");
    
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_WRONG_VALUE, "ctx is null");
  }
  
  ecaio_context_setCallbacks (context, ptr, process, destroy);
  
  return ecaio_addContextToEvent (self, context, err);
}

//-----------------------------------------------------------------------------

int ecaio_appendVNode (EcAio self, int fd, void* data, EcErr err)
{
  int res;
  
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  
  if (data == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "add event", "context is NULL");
    
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_WRONG_VALUE, "ctx is null");
  }
  
  EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE, 0, data);
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_appendPNode (EcAio self, int pid, void* data, EcErr err)
{
  int res;
  
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  
  if (data == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "add event", "context is NULL");
    
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_WRONG_VALUE, "ctx is null");
  }
  
  EV_SET (&kev, pid, EVFILT_PROC, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_EXIT, 0, data);
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_appendENode (EcAio self, EcAioContext ctx, void** eh, EcErr err)
{
  //int res;

  *eh = ctx;
  
  return ENTC_ERR_NONE;
  
  /*
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));
  
  if (ctx == NULL)
  {
    eclogger_fmt (LL_FATAL, "ENTC", "add event", "context is NULL");
    
    return ecerr_set (err, ENTC_LVL_FATAL, ENTC_ERR_WRONG_VALUE, "ctx is null");
  }

  static int identifier = 0;
  
  identifier--;
  *eh = identifier;
  
  EV_SET(&kev, identifier, EVFILT_USER, EV_ADD | EV_ONESHOT, 0, 0, ctx);
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
   */
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_triggerENode (EcAio self, void* eh, EcErr err)
{
  //int res;
  
  /*
  struct kevent kev;
  memset (&kev, 0x0, sizeof(struct kevent));

  eclogger_fmt (LL_TRACE, "ENTC", "event", "trigger event [%i]", eh);
  
  EV_SET (&kev, eh, EVFILT_USER, EV_ENABLE, NOTE_TRIGGER, 0, NULL);
  res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
  if (res < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
   */
  ecaio_context_process (eh, 0, 0);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void ecaio_abort_all (EcAio self)
{
  
}

//-----------------------------------------------------------------------------

int ecaio_wait (EcAio self, unsigned long timeout, EcErr err)
{
  int res;
  struct timespec tmout;
  
  struct kevent event;
  memset (&event, 0x0, sizeof(struct kevent));
  
  if (timeout == ENTC_INFINITE)
  {
    res = kevent (self->kq, NULL, 0, &event, 1, NULL);
  }
  else
  {
    tmout.tv_sec = timeout / 1000;
    tmout.tv_nsec = (timeout % 1000) * 1000;
    
    res = kevent (self->kq, NULL, 0, &event, 1, &tmout);
  }
  
  if( res == -1 )
  {
    if( errno == EINTR )
    {
      return ENTC_ERR_NONE;
    }
    
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  else if (event.flags & EV_ERROR)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  else if (res == 0)
  {
    return ENTC_ERR_NONE;  // timeout
  }
  else
  {
    EcAioContext ctx = event.udata;
    if (ctx)
    {
      int cont = ecaio_context_process (ctx, event.flags, event.filter);
      if (cont == ENTC_AIO_CODE_CONTINUE)
      {
        if (event.ident != -1)
        {
          struct kevent kev;
          EV_SET (&kev, event.ident, EVFILT_READ | EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, ctx);
          
          kevent (self->kq, &kev, 1, NULL, 0, NULL);
        }
      }
      else if (cont == ENTC_AIO_CODE_ABORTALL)
      {
        // terminate and clear everything
        ecaio_abort_all (self);
        
        // abort
        return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_OS_ERROR, "user abborted");
      }
      else if (cont == ENTC_AIO_CODE_ONCE)
      {
        // we don't need to remove the event
        // event was setup as once
      }
      else
      {
        if (event.ident != -1)
        {
          // remove
          struct kevent kev;
          EV_SET (&kev, event.ident, EVFILT_READ | EVFILT_WRITE, EV_DISPATCH, 0, 0, NULL);
          
          res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
          if (res < 0)
          {
            //eclogger_fmt (LL_WARN, "ENTC", "remove", "error in removing event [%i]", event.ident);
          }
          
        }
      }
    }
    else
    {
      const EcString signalKind = NULL;
      
      // assign all known signals
      switch (event.ident)
      {
        case 1: signalKind = "SIGHUP (Hangup detected on controlling terminal or death of controlling process)"; break;
        case 2: signalKind = "SIGINT (Interrupt from keyboard)"; break;
        case 3: signalKind = "SIGQUIT (Quit from keyboard)"; break;
        case 4: signalKind = "SIGILL (Illegal Instruction)"; break;
        case 6: signalKind = "SIGABRT (Abort signal from abort(3))"; break;
        case 8: signalKind = "SIGFPE (Floating-point exception)"; break;
        case 9: signalKind = "SIGKILL (Kill signal)"; break;
        case 11: signalKind = "SIGSEGV (Invalid memory reference)"; break;
        case 13: signalKind = "SIGPIPE (Broken pipe: write to pipe with no readers; see pipe(7))"; break;
        case 15: signalKind = "SIGTERM (Termination signal)"; break;
      }
      
      if (signalKind)
      {
        eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> %s", event.ident, signalKind);
 
        return ecaio_abort (self, err);
      }
      else
      {
        eclogger_fmt (LL_TRACE, "ENTC", "signal", "signal seen [%i] -> unknown signal", event.ident);
      }
    }
    
    return ENTC_ERR_NONE;
  }
}

//-----------------------------------------------------------------------------

static void ecaio_dummy_signalhandler (int signum)
{
}

//-----------------------------------------------------------------------------

int ecaio_wait_abortOnSignal (EcAio self, int onlyTerm, EcErr err)
{
  int res;
  
  if (onlyTerm == FALSE)
  {
    struct kevent kev;
    memset (&kev, 0x0, sizeof(struct kevent));
    
    EV_SET (&kev, SIGINT, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);
    res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
    if (res < 0)
    {
      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
  }
  {
    struct kevent kev;
    memset (&kev, 0x0, sizeof(struct kevent));
    
    EV_SET (&kev, SIGTERM, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);
    res = kevent (self->kq, &kev, 1, NULL, 0, NULL);
    if (res < 0)
    {
      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
  }
  
  signal(SIGTERM, ecaio_dummy_signalhandler);
  signal(SIGINT, ecaio_dummy_signalhandler);
  
  res = ENTC_ERR_NONE;
  while (res == ENTC_ERR_NONE)
  {
    res = ecaio_wait (self, ENTC_INFINITE, err);
  }
  
  return res;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_abort_fct_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  EcAio self = ptr;
  
  // send again
  ecaio_addQueueEvent (self, self, ecaio_abort_fct_process, NULL, NULL);
  
  return ENTC_AIO_CODE_ABORTALL;  // just tell to abort all
}

//-----------------------------------------------------------------------------

int ecaio_abort (EcAio self, EcErr err)
{
  return ecaio_addQueueEvent (self, self, ecaio_abort_fct_process, NULL, err);
}

//*****************************************************************************

#endif

//*****************************************************************************
