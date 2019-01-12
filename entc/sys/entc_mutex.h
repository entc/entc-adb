#ifndef __ENTC_SYS__MUTEX__H
#define __ENTC_SYS__MUTEX__H 1

#include "sys/entc_export.h"
#include "sys/entc_err.h"

//=============================================================================

typedef void* EntcMutex;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcMutex         entc_mutex_new         (void);                // allocate memory and initialize the object

__ENTC_LIBEX   void              entc_mutex_del         (EntcMutex*);          // release memory

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void              entc_mutex_lock        (EntcMutex);

__ENTC_LIBEX   void              entc_mutex_unlock      (EntcMutex);

//-----------------------------------------------------------------------------

#endif



