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
  switch( ctrlType )
  {
    case CTRL_C_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_CLOSE_EVENT:
    {
      EcErr err = ecerr_create ();
      
      int res = ecaio_abort (g_aio, err);
      
      ecerr_destroy (&err);
      
      Sleep (5000);
    }
      break;
  }
  
  return 0;
}

//-----------------------------------------------------------------------------

int ecaio_wait_abortOnSignal (EcAio self, EcErr err)
{
  int res;
  g_aio = self;
  
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
  
};

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
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "append %i", (int)handle, ctx);
  
  int s = epoll_ctl (self->efd, EPOLL_CTL_ADD, hfd, &event);
  if (s < 0)
  {
    int errCode = errno;
    if (errCode == EPERM)
    {
      return ecerr_set(err, ENTC_LVL_ERROR, ENTC_ERR_NOT_SUPPORTED, "file descriptor don't support epoll");
    }
    
    eclogger_fmt (LL_ERROR, "Q6_AIO", "append", "append failed with error %i", s);
    
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

int ecaio_addContextToEvent (EcAio self, EcAioContext ctx, EcErr err)
{
  ecmutex_lock (self->mutex);
  
  eclist_push_back (self->ctxs, ctx);
  
  ecmutex_unlock (self->mutex);
  
  // trigger event
  uint64_t u = 1;
  //write (self->ufd, &u, sizeof(uint64_t));
  write (self->ufd, &u, sizeof(uint64_t));
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "wrote %u", u);
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_addQueueEvent (EcAio self, void* ptr, fct_ecaio_context_process process, fct_ecaio_context_destroy destroy, EcErr err)
{
  // create a new aio context
  EcAioContext ctx = ecaio_context_create ();
  
  ecaio_context_setCallbacks (ctx, ptr, process, destroy);
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "new ctx %p", ctx);
  
  return ecaio_addContextToEvent (self, ctx, err);
}

//-----------------------------------------------------------------------------

int ecaio_wait (EcAio self, unsigned long timeout, EcErr err)
{
  int n;
  struct epoll_event *events;
  
  events = calloc (Q6_EPOLL_MAXEVENTS, sizeof(struct epoll_event));
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "waiting for lock");
  
  ecmutex_lock (self->iom);
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "waiting for event");
  
  n = epoll_wait (self->efd, events, Q6_EPOLL_MAXEVENTS, -1);
  
  //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "got event %i", n);
  
  if (n < 0)
  {
    ecmutex_unlock (self->iom);
    
    free (events);
    
    return ecerr_lastErrorOS(err, ENTC_LVL_ERROR);
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
      
      //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "got ctx from IO event %p", ctx);
      
      int cont = ecaio_context_process (ctx, 0, 0);
      
      //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "return %i", cont);
      
      if (cont == ENTC_AIO_CODE_CONTINUE)
      {
        epoll_ctl (self->efd, EPOLL_CTL_MOD, handle, &(events[i]));
        // continue
      }
      else if (cont == ENTC_AIO_CODE_ABORTALL)
      {
        ecmutex_unlock (self->iom);
        free (events);
        // abort
        return ENTC_ERR_NONE_CONTINUE;
      }
      else
      {
        //eclogger_fmt (LL_TRACE, "Q6_AIO", "context", "remove %i %p", ctx->handle, ctx);
        
        // remove
        struct epoll_event event = {0}; // see bugs
        int s = epoll_ctl (self->efd, EPOLL_CTL_DEL, handle, &event);
        if (s < 0)
        {
          
          
          //eclogger_fmt (LL_ERROR, "Q6_AIO", "wait", "failed to remove file descriptor %i", ctx->handle);
        }
      }
      
      ecmutex_unlock (self->iom);
    }
    else
    {
      ecmutex_unlock (self->iom);
      
      EcAioContext ctx = NULL;
      
      ecmutex_lock (self->mutex);
      
      EcListCursor c;
      eclist_cursor_init (self->ctxs, &c, LIST_DIR_NEXT);
      
      if (eclist_cursor_next (&c))
      {
        ctx = eclist_data(c.node);
        
        eclist_cursor_erase (self->ctxs, &c);
      }
      
      //  int res = read (self->ufd, &u, sizeof(uint64_t));
      //int res = read (self->pfd[0], &u, sizeof(uint64_t));
      
      ecmutex_unlock (self->mutex);
      
      if (ctx)
      {
        // trigger next event
        uint64_t u = 1;
        //write (self->ufd, &u, sizeof(uint64_t));
        write (self->ufd, &u, sizeof(uint64_t));
        
        //eclogger_fmt (LL_TRACE, "Q6_AIO", "event", "context %p", ctx);
        
        int res = ecaio_context_process (ctx, 0, 0);
        
        if (res == ENTC_AIO_CODE_ABORTALL)
        {
          free (events);
          // abort
          return ENTC_ERR_NONE_CONTINUE;
        }
      }
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

static void ecaio_dummy_signalhandler (int signum)
{
  uint64_t u = TRUE;
  write (tfd, &u, sizeof(uint64_t));
}

//-----------------------------------------------------------------------------

static void ecaio_empty_signalhandler (int signum)
{
}

//-----------------------------------------------------------------------------

int ecaio_wait_abortOnSignal (EcAio self, EcErr err)
{
  int res;
  EcAioContext ctx;
  
  signal(SIGTERM, ecaio_dummy_signalhandler);
  signal(SIGINT, ecaio_dummy_signalhandler);
  
  signal(SIGPIPE, ecaio_empty_signalhandler);
  
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
    res = ecaio_wait (self, ENTC_INFINITE, err);
  }
  
  close (tfd);
  
  ecaio_context_destroy(&ctx);
  
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
    eclogger_fmt (LL_FATAL, "Q6_AIO", "add event", "context is NULL");
    
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
    eclogger_fmt (LL_FATAL, "Q6_AIO", "add event", "context is NULL");
    
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
    eclogger_fmt (LL_FATAL, "Q6_AIO", "add event", "context is NULL");
    
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
    eclogger_fmt (LL_FATAL, "Q6_AIO", "add event", "context is NULL");
    
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
        // abort
        return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_OS_ERROR, "user abborted");
      }
      else
      {
        if (event.ident != -1)
        {
          // remove
          struct kevent kev;
          EV_SET (&kev, event.ident, EVFILT_READ | EVFILT_WRITE, EV_DISPATCH, 0, 0, NULL);
          
          kevent (self->kq, &kev, 1, NULL, 0, NULL);
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
        eclogger_fmt (LL_TRACE, "Q6_AIO", "signal", "signal seen [%i] -> %s", event.ident, signalKind);
      }
      else
      {
        eclogger_fmt (LL_TRACE, "Q6_AIO", "signal", "signal seen [%i] -> unknown signal", event.ident);
      }
      
      return ecaio_abort (self, err);
      
      /*
      if (event.ident == SIGINT || event.ident == SIGTERM)
      {
        return ENTC_ERR_NONE_CONTINUE;
      }
      
      // abort
      return ecerr_set (err, ENTC_LVL_ERROR, ENTC_ERR_OS_ERROR, "wait abborted (signal?)");
       */
    }
    
    return ENTC_ERR_NONE;
  }
}

//-----------------------------------------------------------------------------

static void ecaio_dummy_signalhandler (int signum)
{
}

//-----------------------------------------------------------------------------

int ecaio_wait_abortOnSignal (EcAio self, EcErr err)
{
  int res;
  
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
