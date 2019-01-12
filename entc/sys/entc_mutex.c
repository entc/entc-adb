#include "entc_mutex.h"
#include "sys/entc_types.h"

#include <pthread.h>

//-----------------------------------------------------------------------------

EntcMutex entc_mutex_new (void)
{
  pthread_mutex_t* self = ENTC_NEW(pthread_mutex_t);
  
  pthread_mutex_init (self, NULL);
  
  return self;
}

//-----------------------------------------------------------------------------

void entc_mutex_del (EntcMutex* p_self)
{
  pthread_mutex_t* self = *p_self;
  
  pthread_mutex_destroy (self);

  ENTC_DEL(p_self, pthread_mutex_t);  
}

//-----------------------------------------------------------------------------

void entc_mutex_lock (EntcMutex self)
{
  pthread_mutex_lock (self);
}

//-----------------------------------------------------------------------------

void entc_mutex_unlock (EntcMutex self)
{
  pthread_mutex_unlock (self);
}

//-----------------------------------------------------------------------------
