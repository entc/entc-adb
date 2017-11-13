#include "ecaio.h"

#include <stdio.h>

// entc inlcudes
#include "system/macros.h"
#include "system/ecmutex.h"
#include "types/ecbuffer.h"
#include "utils/eclogger.h"

#if defined __LINUX_EPOLL || defined __BSD_KEVENT

#define Q6HANDLE long

#include <errno.h>
#include <unistd.h>


//=============================================================================

#if defined __LINUX_EPOLL

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

#elif defined __BSD_KEVENT

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

#endif



//-----------------------------------------------------------------------------

#endif

