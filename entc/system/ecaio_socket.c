#include "ecaio_socket.h"

#define READ_MAX_BUFFER 1024

// entc includes
#include <system/macros.h>
#include <utils/eclogger.h>
#include <utils/ecmessages.h>

//*****************************************************************************

#if defined __MS_IOCP

//*****************************************************************************

#include <WinSock2.h>
#include <Mswsock.h>
#include <WS2tcpip.h>

#include "q6sys_sock.h"
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <stdio.h>

//=============================================================================

static int init_wsa (void)
{
  static WSADATA wsa;
  
  return (WSAStartup(MAKEWORD(2,2), &wsa) == 0);
}

//=============================================================================

struct EcAcceptSocket_s
{
  
  SOCKET handle;
  
};

//-----------------------------------------------------------------------------

EcAcceptSocket ecacceptsocket_create ()
{
  EcAcceptSocket self = ENTC_NEW(struct EcAcceptSocket_s);
  
  self->handle = INVALID_SOCKET;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecacceptsocket_destroy (EcAcceptSocket* pself)
{
  EcAcceptSocket self = *pself;
  
  closesocket(self->handle);
  
  ENTC_DEL(pself, struct EcAcceptSocket_s);
}

//-----------------------------------------------------------------------------

int ecacceptsocket_listen (EcAcceptSocket self, const char* host, int port, Q6Err err)
{
  struct addrinfo hints;
  struct addrinfo* addr;
  SOCKET sock;
  
  if (!init_wsa ())
  {
    return q6err_set (err, Q6LVL_FATAL, Q6ERR_OS_ERROR, "can't initialize Windows WSA");
  }
  
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;
  
  {
    char buffer[6];
    sprintf_s (buffer, 6, "%u", port);
    
    if (getaddrinfo(host, buffer, &hints, &addr) != 0)
    {
      return q6err_formatErrorOS (err, Q6LVL_ERROR, WSAGetLastError());
    }
  }
  
  sock = socket (addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if (sock == INVALID_SOCKET)
  {
    freeaddrinfo(addr);
    return q6err_set (err, Q6LVL_ERROR, Q6ERR_OS_ERROR, "can't create socket");
  }
  
  {
    int optVal = TRUE;
    int optLen = sizeof(int);
    
    if (getsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, &optLen) != 0)
    {
      return q6err_set (err, Q6LVL_ERROR, Q6ERR_OS_ERROR, "can't apply options");
    }
  }
  
  if (bind(sock, addr->ai_addr, (int)addr->ai_addrlen) == SOCKET_ERROR)
  {
    freeaddrinfo(addr);
    closesocket(sock);
    return q6err_set (err, Q6LVL_ERROR, Q6ERR_OS_ERROR, "can't bind");
  }
  
  freeaddrinfo(addr);
  
  if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
  {
    closesocket(sock);
    return q6err_set (err, Q6LVL_ERROR, Q6ERR_OS_ERROR, "can't listen");
  }
  
  self->handle = sock;
  
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

void* ecacceptsocket_socket (EcAcceptSocket self)
{
  return (void*)self->handle;
}

//=============================================================================

struct EcRefCountedSocket_s
{
  
  SOCKET handle;
  
  LONG cnt;
  
};

//-----------------------------------------------------------------------------

EcRefCountedSocket ecrefsocket_create (void* socket)
{
  EcRefCountedSocket self = ENTC_NEW(struct EcRefCountedSocket_s);
  
  self->handle = (SOCKET)socket;
  self->cnt = 1;
  
  return self;
}

//-----------------------------------------------------------------------------

EcRefCountedSocket ecrefsocket_clone (EcRefCountedSocket self)
{
  InterlockedIncrement (&(self->cnt));
  
  return self;
}

//-----------------------------------------------------------------------------

void ecrefsocket_decrease (EcRefCountedSocket* pself)
{
  EcRefCountedSocket self = *pself;
  
  int var = InterlockedDecrement (&(self->cnt));
  if (var == 0)
  {
    closesocket ((SOCKET)self->handle);
    
    ENTC_DEL (&self, struct EcRefCountedSocket_s);
  }
  
  *pself = NULL;
}

//-----------------------------------------------------------------------------

void* ecrefsocket_socket (EcRefCountedSocket self)
{
  return (void*)self->handle;
}

//=============================================================================

struct EcAioSocketReader_s
{
  SOCKET handle;
  
  char buffer [READ_MAX_BUFFER];
  
  fct_aio_context_onRead onRead;
  
  fct_aio_context_destroy destroy;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioSocketReader ecaio_socketreader_create (void* handle)
{
  EcAioSocketReader self = ENTC_NEW(struct EcAioSocketReader_s);
  
  //printf ("filewriter created on handle %p\r\n", handle);
  
  self->handle = (SOCKET)handle;
  
  // callbacks
  self->destroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_socketreader_fct_destroy (void* ptr)
{
  EcAioSocketReader self = ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  ENTC_DEL (&self, struct EcAioSocketReader_s);
}

//-----------------------------------------------------------------------------

int ecaio_socketreader_read (EcAioSocketReader self, Q6AIOContext ctx)
{
  DWORD dwFlags = 0;
  DWORD dwBytes = 0;
  WSABUF dataBuf;
  int nBytesRecv;
  
  dataBuf.buf = self->buffer;
  dataBuf.len = 1024;
  
  nBytesRecv = WSARecv((unsigned int)self->handle, &dataBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)ctx->overlapped, NULL);
  if (nBytesRecv == SOCKET_ERROR)
  {
    DWORD res = WSAGetLastError();
    if (res == WSA_IO_PENDING || res == 997)
    {
      return OVL_PROCESS_CODE_CONTINUE;
    }
    
    //TError err (_ERRCODE_NET__SOCKET_ERROR, "error code: " + IntToStr (::WSAGetLastError()));
    
    //std::cout << err.getText().c_str() << std::endl;
    return OVL_PROCESS_CODE_DONE;
  }
  else if (nBytesRecv == 0)
  {
    //std::cout << "connection probably closed" << std::endl;
  }
  
  return OVL_PROCESS_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __stdcall ecaio_socketreader_fct_process (void* ptr, Q6AIOContext ctx, unsigned long len, unsigned long opt)
{
  EcAioSocketReader self = ptr;
  
  if (len == 0)
  {
    return OVL_PROCESS_CODE_DONE;
  }
  
  if (self->onRead)
  {
    if (self->onRead (self->ptr, (void*)self->handle, self->buffer, len))
    {
      return OVL_PROCESS_CODE_DONE;
    }
  }
  
  return ecaio_socketreader_read (self, ctx);
}

//-----------------------------------------------------------------------------

void ecaio_socketreader_setCallback (EcAioSocketReader self, void* ptr, fct_aio_context_onRead onRead, fct_aio_context_destroy destroy)
{
  self->ptr = ptr;
  self->onRead = onRead;
  self->destroy = destroy;
}

//-----------------------------------------------------------------------------

int ecaio_socketreader_assign (EcAioSocketReader* pself, Q6AIO aio, Q6Err err)
{
  int res;
  EcAioSocketReader self = *pself;
  
  res = q6sys_aio_append (aio, (void*)self->handle, NULL, err);
  if (res)
  {
    return res;
  }
  
  {
    // create a async context
    EcAioContext ctx = ecaio_context_create ();
    
    // override callbacks
    ecaio_context_setCallbacks (ctx, self, ecaio_socketreader_fct_process, ecaio_socketreader_fct_destroy);
    
    // assign this and the context to the async system
    ecaio_socketreader_read (self, ctx);
  }
  
  *pself = NULL;
  return Q6ERR_NONE;
}

//=============================================================================

struct EcAioSocketWriter_s
{
  
  EcRefCountedSocket refSocket;
  
  fct_aio_context_destroy destroy;
  
  void* ptr;
  
  unsigned long written;
  
  unsigned long offset;
  
  EcBuffer buf;
  
};

//-----------------------------------------------------------------------------

EcAioSocketWriter ecaio_socketwriter_create (EcRefCountedSocket refSocket)
{
  EcAioSocketWriter self = ENTC_NEW(struct EcAioSocketWriter_s);
  
  //printf ("filewriter created on handle %p\r\n", handle);
  
  self->refSocket = ecrefsocket_clone (refSocket);
  self->written = 0;
  
  // buffer
  self->buf = NULL;
  
  // callbacks
  self->destroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_socketwriter_fct_destroy (void* ptr)
{
  EcAioSocketWriter self = ptr;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  ecrefsocket_decrease (&(self->refSocket));
  
  ENTC_DEL(&self, struct EcAioSocketWriter_s);
}

//-----------------------------------------------------------------------------

int ecaio_socketwriter_write (EcAioSocketWriter self, Q6AIOContext ctx)
{
  DWORD dwFlags = 0;
  DWORD dwBytes = 0;
  WSABUF dataBuf;
  int res;
  
  dataBuf.buf = self->buf->buffer + self->written;
  dataBuf.len = self->buf->size - self->written;
  
  //printf ("sw: '%s'", self->buf->buffer + self->written);
  
  res = WSASend ((unsigned int)ecrefsocket_socket (self->refSocket), &dataBuf, 1, &dwBytes, dwFlags, (WSAOVERLAPPED*)ctx->overlapped, NULL);
  if (res == 0)
  {
    //std::cout << "connection probably closed" << std::endl;
    //return false;
  }
  else if (res < 0)
  {
    DWORD err = GetLastError ();
    if (err != ERROR_IO_PENDING)
    {
      return OVL_PROCESS_CODE_DONE;
    }
  }
  else
  {
    
  }
  
  return OVL_PROCESS_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

static int __stdcall ecaio_socketwriter_fct_process (void* ptr, Q6AIOContext ctx, unsigned long len, unsigned long opt)
{
  EcAioSocketWriter self = ptr;
  
  self->written += len;
  
  if (self->written < self->buf->size)
  {
    return ecaio_socketwriter_write (self, ctx);
  }
  else
  {
    return OVL_PROCESS_CODE_DONE;
  }
}

//-----------------------------------------------------------------------------

int ecaio_socketwriter_assign (EcAioSocketWriter* pself, Q6Err err)
{
  EcAioSocketWriter self = *pself;
  
  // create a async context
  Q6AIOContext ctx = q6sys_aio_context_create ();
  
  // override callbacks
  q6sys_aio_context_setCallbacks (ctx, self, ecaio_socketwriter_fct_process, ecaio_socketwriter_fct_destroy);
  
  // assign this and the context to the async system
  ecaio_socketwriter_write (self, ctx); // == OVL_PROCESS_CODE_CONTINUE;
  
  *pself = NULL;
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

void ecaio_socketwriter_setBufferCP (EcAioSocketWriter self, const char* buffer, unsigned long size)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = ecbuf_create_fromBuffer (buffer, size);
}

//-----------------------------------------------------------------------------

void ecaio_socketwriter_setBufferBT (EcAioSocketWriter self, EcBuffer* buf)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = *buf;
  *buf = NULL;
}

//=============================================================================

struct EcAioSocketAccept_s
{
  SOCKET handle;
  
  fct_aio_socket_accept accept;
  
  void* ptr;
  
  char buffer[1024];
  
  SOCKET asock;
  
};

//-----------------------------------------------------------------------------

EcAioSocketAccept ecaio_socketaccept_create (void* handle)
{
  EcAioSocketAccept self = ENTC_NEW(struct EcAioSocketAccept_s);
  
  //printf ("filewriter created on handle %p\r\n", handle);
  
  self->handle = (SOCKET)handle;
  
  // buffer
  memset (self->buffer, 0, 1024);
  self->asock = INVALID_SOCKET;
  
  // callbacks
  self->accept = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __stdcall ecaio_socketaccept_fct_destroy (void* ptr)
{
  EcAioSocketAccept self = ptr;
  
  ENTC_DEL(&self, struct EcAioSocketAccept_s);
}

//-----------------------------------------------------------------------------

void ecaio_socketaccept_setCallback (EcAioSocketAccept self, void* ptr, fct_aio_socket_accept accept)
{
  self->accept = accept;
  self->ptr = ptr;
}

//-----------------------------------------------------------------------------

static const DWORD lenAddr = sizeof (struct sockaddr_in) + 16;

//-----------------------------------------------------------------------------

int ecaio_socketaccept_accept (EcAioSocketAccept self, Q6AIOContext ctx)
{
  DWORD outBUflen = 0; //1024 - ((sizeof (sockaddr_in) + 16) * 2);
  DWORD dwBytes = 0;
  
  // create a client socket in advance
  self->asock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (self->asock == INVALID_SOCKET)
  {
    return Q6ERR_OS_ERROR;
    //return TError (_ERRCODE_NET__SOCKET_ERROR, "error code: " + IntToStr (::WSAGetLastError()));
  }
  
  memset (self->buffer, 0, 1024);
  
  // accept connections, client socket will be assigned
  AcceptEx (self->handle, self->asock, self->buffer, outBUflen, lenAddr, lenAddr, &dwBytes, (LPOVERLAPPED)ctx->overlapped);
  
  return Q6ERR_NONE;
}

//-----------------------------------------------------------------------------

static int __stdcall ecaio_socketaccept_fct_process (void* ptr, Q6AIOContext ctx, unsigned long len, unsigned long opt)
{
  EcAioSocketAccept self = ptr;
  
  struct sockaddr *pLocal = NULL, *pRemote = NULL;
  int nLocal = 0, nRemote = 0;
  const char* remoteAddr = NULL;
  
  // retrieve the remote address
  GetAcceptExSockaddrs (self->buffer, 0, lenAddr, lenAddr, &pLocal, &nLocal, &pRemote, &nRemote);
  
  if (pRemote->sa_family == AF_INET)
  {
    remoteAddr = inet_ntoa(((struct sockaddr_in*)pRemote)->sin_addr);
  }
  
  if (self->accept)
  {
    if (self->accept (self->ptr, (void*)self->asock, remoteAddr))
    {
      // abort connection
      closesocket (self->asock);
    }
  }
  
  self->asock = INVALID_SOCKET;
  
  // continue
  ecaio_socketaccept_accept (self, ctx);
  
  return OVL_PROCESS_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

int ecaio_socketaccept_assign (EcAioSocketAccept* pself, Q6AIO aio, Q6Err err)
{
  EcAioSocketAccept self = *pself;
  
  {
    // link socket with io port
    int res = q6sys_aio_append (aio, (void*)self->handle, NULL, err);
    if (res != Q6ERR_NONE)
    {
      return res;
    }
  }
  {
    // create a async context
    Q6AIOContext ctx = q6sys_aio_context_create ();
    
    // override callbacks
    q6sys_aio_context_setCallbacks (ctx, self, ecaio_socketaccept_fct_process, ecaio_socketaccept_fct_destroy);
    
    // assign this and the context to the async system
    ecaio_socketaccept_accept (self, ctx); // == OVL_PROCESS_CODE_CONTINUE;
  }
  
  *pself = NULL;
  return Q6ERR_NONE;
}

//*****************************************************************************

#else

//*****************************************************************************

#if defined __APPLE__
#include <stdatomic.h>
#endif

#define Q6HANDLE long

#include <sys/socket.h>	// basic socket definitions
#include <sys/types.h>
#include <arpa/inet.h>	// inet(3) functions
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

// entc includes
#include <system/ecfile.h>

//=============================================================================

struct EcAcceptSocket_s
{
  
  long socket;
  
};

//-----------------------------------------------------------------------------

EcAcceptSocket ecacceptsocket_create ()
{
  EcAcceptSocket self = ENTC_NEW(struct EcAcceptSocket_s);
  
  self->socket = -1;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecacceptsocket_destroy (EcAcceptSocket* pself)
{
  //EcAcceptSocket self = *pself;
  
  ENTC_DEL(pself, struct EcAcceptSocket_s);
}

//-----------------------------------------------------------------------------

int ecacceptsocket_listen (EcAcceptSocket self, const char* host, int port, EcErr err)
{
  struct sockaddr_in addr;
  struct hostent* server;
  int sock;
  
  memset ( &addr, 0, sizeof(addr) );
  /* set the address */
  addr.sin_family = AF_INET;
  /* set the port */
  addr.sin_port = htons(port);
  /* set the address */
  if (ecstr_empty(host))
  {
    addr.sin_addr.s_addr = INADDR_ANY;
  }
  else
  {
    server = gethostbyname(host);
    
    if(server)
    {
      bcopy((char*)server->h_addr, (char*)&(addr.sin_addr.s_addr), server->h_length);
    }
    else
    {
      addr.sin_addr.s_addr = INADDR_ANY;
    }
  }
  
  // create socket
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
  }
  
  {
    int opt = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
      close(sock);
      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
    
    if (bind(sock, (const struct sockaddr*)&(addr), sizeof(addr)) < 0)
    {
      close(sock);
      return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
    }
  }
  
  // cannot fail
  listen(sock, SOMAXCONN);
  
  self->socket = sock;
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

void* ecacceptsocket_socket (EcAcceptSocket self)
{
  return (void*)self->socket;
}

//=============================================================================

struct EcRefCountedSocket_s
{
  
  Q6HANDLE socket;
  
  /*
   #if defined __APPLE__
   
   atomic_int cnt;
   
   #else
   */
  int cnt;
  
  //#endif
  
};

//-----------------------------------------------------------------------------

EcRefCountedSocket ecrefsocket_create (void* socket)
{
  EcRefCountedSocket self = ENTC_NEW(struct EcRefCountedSocket_s);
  
  self->socket = (Q6HANDLE)socket;
  self->cnt = 1;
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "refsock", "create [%p] %i", self, self->socket);
  
  return self;
}

//-----------------------------------------------------------------------------

EcRefCountedSocket ecrefsocket_clone (EcRefCountedSocket self)
{
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
  
  //#endif
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "refsock", "clone [%p] %i", self, self->socket);
  
  return self;
}

//-----------------------------------------------------------------------------

void ecrefsocket_decrease (EcRefCountedSocket* pself)
{
  EcRefCountedSocket self = *pself;
  
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
  
  //#endif
  
  if (val == 0)
  {
    //eclogger_fmt (LL_TRACE, "Q6_SOCK", "swriter", "shutdown writing [%i]", self->socket);
    
    shutdown (self->socket, SHUT_WR);
    
    //eclogger_fmt (LL_TRACE, "Q6_SOCK", "refsock", "close [%p] socket %i", self, self->socket);
    
    close (self->socket);
    
    ENTC_DEL (&self, struct EcRefCountedSocket_s);
  }
  
  *pself = NULL;
}

//-----------------------------------------------------------------------------

void* ecrefsocket_socket (EcRefCountedSocket self)
{
  return (void*)self->socket;
}

//=============================================================================

struct EcAioSocketReader_s
{
  Q6HANDLE socket;
  
  char buffer [READ_MAX_BUFFER];
  
  fct_ecaio_context_onRead onRead;
  
  fct_ecaio_context_destroy destroy;
  
  fct_ecaio_context_onInit onInit;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioSocketReader ecaio_socketreader_create (void* handle)
{
  EcAioSocketReader self = ENTC_NEW(struct EcAioSocketReader_s);
  
  //printf ("filewriter created on handle %p\r\n", handle);
  
  self->socket = (Q6HANDLE)handle;
  
  // callbacks
  self->onRead = NULL;
  self->onInit = NULL;
  self->destroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_socketreader_fct_destroy (void* ptr)
{
  EcAioSocketReader self = ptr;
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sreader", "destroy");
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "sreader", "shutdown reading [%i]", self->socket);
  
  shutdown (self->socket, SHUT_RD);
  
  ENTC_DEL (&self, struct EcAioSocketReader_s);
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_socketreader_fct_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  int count;
  
  EcAioSocketReader self = ptr;
  
  count = read (self->socket, self->buffer, READ_MAX_BUFFER);
  if (count == -1)
  {
    if (errno != EAGAIN) // If errno == EAGAIN, that means we have read all
    {
      return ENTC_AIO_CODE_CONTINUE;
    }
    
    return ENTC_AIO_CODE_DONE;
  }
  else if (count == 0)
  {
    return ENTC_AIO_CODE_DONE;
  }
  
  if (self->onRead)
  {
    int res = self->onRead (self->ptr, (void*)self->socket, self->buffer, count);
    if (res != ENTC_ERR_NONE)
    {
      return ENTC_AIO_CODE_DONE;
    }
  }
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

void ecaio_socketreader_setCallback (EcAioSocketReader self, void* ptr, fct_ecaio_context_onRead onRead, fct_ecaio_context_destroy destroy)
{
  self->ptr = ptr;
  self->onRead = onRead;
  self->destroy = destroy;
}

//-----------------------------------------------------------------------------

int ecaio_socketreader_assign (EcAioSocketReader* pself, EcAio aio, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcAioSocketReader self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_socketreader_fct_process, ecaio_socketreader_fct_destroy);
  
  res = ecaio_append (aio, (void*)self->socket, ctx, err);
  if (res)
  {
    ecaio_context_destroy (&ctx);
    return res;
  }
  
  if (self->onInit)
  {
    res = self->onInit (self, err);
    if (res)
    {
      ecaio_context_destroy (&ctx);
    }
  }
  
  *pself = NULL;
  return res;
}

//=============================================================================

struct EcAioSocketWriter_s
{
  
  EcRefCountedSocket refSocket;
  
  fct_ecaio_context_destroy destroy;
  
  void* ptr;
  
  EcBuffer buf;
  
};

//-----------------------------------------------------------------------------

EcAioSocketWriter ecaio_socketwriter_create (EcRefCountedSocket refSock)
{
  EcAioSocketWriter self = ENTC_NEW(struct EcAioSocketWriter_s);
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "swriter", "create [%p]", refSock);
  
  self->refSocket = ecrefsocket_clone (refSock);
  
  // buffer
  self->buf = NULL;
  
  // callbacks
  self->destroy = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

void ecaio_socketwriter_destroy (EcAioSocketWriter* pself)
{
  EcAioSocketWriter self = *pself;
  
  if (self->destroy)
  {
    self->destroy (self->ptr);
  }
  
  if (self->buf)
  {
    ecbuf_destroy(&(self->buf));
  }
  
  ecrefsocket_decrease (&(self->refSocket));
  
  ENTC_DEL(pself, struct EcAioSocketWriter_s);
}

//-----------------------------------------------------------------------------

int ecaio_socketwriter_send (EcAioSocketWriter self, EcErr err)
{
  Q6HANDLE sock = (Q6HANDLE)ecrefsocket_socket (self->refSocket);
  
  int del = 0;
  while (del < self->buf->size)
  {
    //eclogger_fmt (LL_TRACE, "Q6_SOCK", "swriter", "assign [%p] %u", self->refSocket, sock);
    
    int res = send (sock, (char*)self->buf->buffer + del, self->buf->size - del, 0);
    if (res < 0)
    {
      if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
      {
        return ecerr_lastErrorOS (err, ENTC_LVL_ERROR);
      }
      else
      {
        continue;
      }
    }
    else if (res == 0)
    {
      break;
    }
    else
    {
      del += res;
    }
  }
  
  return ENTC_ERR_NONE;
}

//-----------------------------------------------------------------------------

int ecaio_socketwriter_assign (EcAioSocketWriter* pself, EcErr err)
{
  // send all data to socket
  int ret = ecaio_socketwriter_send (*pself, err);
  
  // clean up
  ecaio_socketwriter_destroy (pself);
  
  return ret;
}

//-----------------------------------------------------------------------------

void ecaio_socketwriter_setBufferCP (EcAioSocketWriter self, const char* buffer, unsigned long size)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = ecbuf_create_fromBuffer ((const unsigned char*)buffer, size);
}

//-----------------------------------------------------------------------------

void ecaio_socketwriter_setBufferBT (EcAioSocketWriter self, EcBuffer* buf)
{
  if (self->buf)
  {
    ecbuf_destroy (&(self->buf));
  }
  
  self->buf = *buf;
  *buf = NULL;
}

//=============================================================================

struct EcAioSocketAccept_s
{
  
  Q6HANDLE socket;
  
  fct_aio_socket_accept accept;
  
  void* ptr;
  
};

//-----------------------------------------------------------------------------

EcAioSocketAccept ecaio_socketaccept_create (void* handle)
{
  EcAioSocketAccept self = ENTC_NEW(struct EcAioSocketAccept_s);
  
  //printf ("filewriter created on handle %p\r\n", handle);
  
  self->socket = (Q6HANDLE)handle;
  
  // callbacks
  self->accept = NULL;
  self->ptr = NULL;
  
  return self;
}

//-----------------------------------------------------------------------------

static void __STDCALL ecaio_socketaccept_fct_destroy (void* ptr)
{
  EcAioSocketAccept self = ptr;
  
  ENTC_DEL(&self, struct EcAioSocketAccept_s);
}

//-----------------------------------------------------------------------------

void ecaio_socketaccept_setCallback (EcAioSocketAccept self, void* ptr, fct_aio_socket_accept accept)
{
  self->accept = accept;
  self->ptr = ptr;
}

//-----------------------------------------------------------------------------

static int __STDCALL ecaio_socketaccept_fct_process (void* ptr, EcAioContext ctx, unsigned long val1, unsigned long val2)
{
  EcAioSocketAccept self = ptr;
  
  struct sockaddr addr;
  socklen_t addrlen = 0;
  
  const char* remoteAddr = NULL;
  
  memset (&addr, 0x00, sizeof(addr));
  
  long sock = accept (self->socket, &addr, &addrlen);
  if (sock < 0)
  {
    if( (errno != EWOULDBLOCK) && (errno != EINPROGRESS) && (errno != EAGAIN))
    {
      eclogger_fmt (LL_TRACE, "Q6_SOCK", "accept", "socket done");
      
      return ENTC_AIO_CODE_DONE;
    }
    else
    {
      return ENTC_AIO_CODE_CONTINUE;
    }
  }
  
  remoteAddr = inet_ntoa(((struct sockaddr_in*)&addr)->sin_addr);
  
  //eclogger_fmt (LL_TRACE, "Q6_SOCK", "accept", "new connection %s | %i", remoteAddr, sock);
  
  if (self->accept)
  {
    if (self->accept (self->ptr, (void*)sock, remoteAddr))
    {
      eclogger_fmt (LL_WARN, "Q6_SOCK", "accept", "connection dropped");
      
      // abort connection
      close (sock);
    }
  }
  
  return ENTC_AIO_CODE_CONTINUE;
}

//-----------------------------------------------------------------------------

int ecaio_socketaccept_assign (EcAioSocketAccept* pself, EcAio aio, EcErr err)
{
  int res = ENTC_ERR_NONE;
  EcAioSocketAccept self = *pself;
  
  // create a async context
  EcAioContext ctx = ecaio_context_create ();
  
  // override callbacks
  ecaio_context_setCallbacks (ctx, self, ecaio_socketaccept_fct_process, ecaio_socketaccept_fct_destroy);
  
  res = ecaio_append (aio, (void*)self->socket, ctx, err);
  
  *pself = NULL;
  return res;
}

//-----------------------------------------------------------------------------

#endif
