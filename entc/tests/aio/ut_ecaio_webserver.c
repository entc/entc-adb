#include "aio/ecaio.h"
#include "aio/ecaio_socket.h"
#include "aio/ecaio_sendf.h"

#include "tests/ecenv.h"

#include "sys/entc_thread.h"
#include "tools/eclparser.h"
#include "types/ecstream.h"
#include "tools/eclog.h"

#include <stdio.h>

//=============================================================================

static void* __STDCALL test_ecaio_init (EcErr err)
{
  EcAio aio = ecaio_create ();
  
  ecaio_init (aio, err);
  
  return aio;
}

//---------------------------------------------------------------------------

static void __STDCALL test_ecaio_done (void* ptr)
{
  EcAio aio = ptr;
    
  ecaio_destroy (&aio);
}

//---------------------------------------------------------------------------

typedef struct
{
  
  EcAio aio;
  
  EcLineParser lp;
  
  int done;
  
  EcRefCountedSocket refsock;
  
} TestParserCtx;

//---------------------------------------------------------------------------

static void __STDCALL text_ecaio_reader_onLine (void* ptr, const EcString line)
{
  TestParserCtx* ctx = ptr;
  
  if (line == NULL)  // last line of the http header
  {
    ctx->done = TRUE;
  }
  else
  {
    //printf ("LINE: %s\n", line);
  }
}

//---------------------------------------------------------------------------

static int __STDCALL text_ecaio_sendfile_onInit (void* ptr, EcRefCountedSocket refsock, uint64_t fileSize, const EcString file, const EcString name, EcErr err)
{
  EcBuffer buf;
  EcAioSocketWriter aioWriter;

  // add http header
  EcStream stream = ecstream_create ();
  
  ecstream_append_str(stream, "HTTP/1.0 200 OK\n");
  ecstream_append_str(stream, "Connection: close\n");
  ecstream_append_str(stream, "Content-Type: text/html\n");
  ecstream_append_str(stream, "Content-Size: ");
  ecstream_append_u64(stream, fileSize);
  ecstream_append_str(stream, "\n\n");
  
  // convert to buffer
  buf = ecstream_tobuf (&stream);
  
  aioWriter = ecaio_socketwriter_create (refsock);

  ecaio_socketwriter_setBufferBT (aioWriter, &buf);  

  ecaio_socketwriter_assign (&aioWriter, err);
  
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static void __STDCALL text_ecaio_sendfile_onDone (void* ptr, const EcString file, const EcString name)
{

}

//---------------------------------------------------------------------------

static int __STDCALL text_ecaio_reader_onRead (void* ptr, void* handle, const char* buffer, unsigned long size)
{
  TestParserCtx* ctx = ptr;

  eclog_fmt (LL_TRACE, "TEST", "reading", "parse");

  // we don't know if this was the last line
  eclineparser_parse (ctx->lp, buffer, size, FALSE);
  
  if (ctx->done)
  {
    EcAioSendFile sf;

    eclog_fmt (LL_TRACE, "TEST", "reading", "got request");
    
    // last line seen, send content
    sf = ecaio_sendfile_create ("index.html", NULL, ctx->refsock, NULL, text_ecaio_sendfile_onInit, text_ecaio_sendfile_onDone);
    
    // transfer ownership to aio
    ecaio_sendfile_assign (&sf, ctx->aio, NULL);
    
    return ENTC_ERR_NONE_CONTINUE;
  }
  else
  {
    return ENTC_ERR_NONE;
  }
}

//---------------------------------------------------------------------------

static void __STDCALL text_ecaio_reader_onDestroy (void* ptr)
{
  TestParserCtx* ctx = ptr;
  
  // tell that the reader don't need the socket anymore
  ecrefsocket_decrease (&(ctx->refsock));
  
  // clean up line parser
  eclineparser_destroy (&(ctx->lp));
  
  ENTC_DEL(&ctx, TestParserCtx);
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_onAccept (void* ptr, void* socket, const char* remoteAddress)
{
  EcAioSocketReader readerAio;

  EcAio aio = ptr;
  
  //printf ("accept connection: %s\n", remoteAddress);
  
  eclog_fmt (LL_TRACE, "TEST", "accept", "accept connection: %s", remoteAddress);
  
  // create an aio reader on the socket
  readerAio = ecaio_socketreader_create (socket);

  {
    // create the 'parser' context  
    TestParserCtx* ctx = ENTC_NEW(TestParserCtx);
    
    ctx->aio = aio;
    ctx->done = FALSE;
    
    // create a new line parser
    ctx->lp = eclineparser_create (text_ecaio_reader_onLine, ctx);
        
    // create a referenced counted socket
    ctx->refsock = ecrefsocket_create (socket);
    
    ecaio_socketreader_setCallback (readerAio, ctx, text_ecaio_reader_onRead, text_ecaio_reader_onDestroy);
  }

  {
    EcErr err = ecerr_create();
    
    int res = ecaio_socketreader_assign (&readerAio, aio, err);
    if (res)
    {
      
    }
    
    ecerr_destroy(&err);
  }
  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_worker (void* ptr)
{
  int res;
  EcErr err = ecerr_create();
  
  EcAio aio = ptr;
  
  eclog_fmt (LL_TRACE, "TEST", "start", "wait for AIO events");
  
  res = ecaio_wait (aio, err);

  ecerr_destroy (&err);
  
  return 0;
}

//---------------------------------------------------------------------------

static int __STDCALL test_ecaio_test1 (void* ptr, TestEnvContext ctx, EcErr err)
{
  int res;
  EcAioSocketAccept acceptAio;

  EcAio aio = ptr;
  
  // create accept socket
  EcAcceptSocket acceptSocket = ecacceptsocket_create ();

  //eclog_fmt (LL_TRACE, "TEST", "start", "about to create the listen socket");
  
  res = ecacceptsocket_listen (acceptSocket, "127.0.0.1", 8080, err);
  if (res)
  {
    return res;
  }
  
  //eclog_fmt (LL_TRACE, "TEST", "start", "link socket to AIO");

  acceptAio = ecaio_socketaccept_create (ecacceptsocket_socket (acceptSocket));

  ecaio_socketaccept_setCallback (acceptAio, aio, test_ecaio_onAccept);

  res = ecaio_socketaccept_assign (&acceptAio, aio, err);
  if (res)
  {
    return res;
  }
  
  res = ecaio_registerTerminateControls (aio, FALSE, err);
  if (res)
  {
    return res;
  }
  
  {
    int i;
    EntcThread threads [16];
    
    for (i = 0; i < 16; i++)
    {
      threads [i] = entc_thread_new ();
     
      entc_thread_start (threads [i], test_ecaio_worker, aio);
    }
    
    for (i = 0; i < 16; i++)
    {
      entc_thread_join (threads [i]);
      
      entc_thread_del (&(threads[i]));
    }
  }    
    
  eclog_fmt (LL_TRACE, "TEST", "start", "wait done");

  ecacceptsocket_destroy (&acceptSocket);

  return 0;
}

//=============================================================================

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "Main", test_ecaio_init, test_ecaio_done, test_ecaio_test1);
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
