#include "ecalloc.h"

#include "sys/entc_types.h"

//-------------------------------------------------------------------------------------------

void* __STDCALL EC_NEW (void* ptr, uint32_t size)
{
  return ENTC_MALLOC(size);
}

//-------------------------------------------------------------------------------------------

void __STDCALL EC_DEL (void* ptr, void** pobj, uint32_t size)
{
  // clear all data
  memset (*pobj, 0, size);
  
  // free
  ENTC_FREE(*pobj);
  
  // return
  *pobj = NULL;
}

//-------------------------------------------------------------------------------------------
