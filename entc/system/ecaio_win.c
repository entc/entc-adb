#include "ecaio.h"

#include <stdio.h>

// entc inlcudes
#include "system/macros.h"
#include "types/ecbuffer.h"
#include "system/ecmutex.h"
#include "utils/eclogger.h"

#define READ_MAX_BUFFER 1024

#if defined __MS_IOCP

//=============================================================================



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
