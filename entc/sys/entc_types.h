#ifndef __ENTC_SYS__TYPES__H
#define __ENTC_SYS__TYPES__H 1

#include <stdlib.h>
#include <memory.h>

//-----------------------------------------------------------------------------

#define u_t unsigned
#define number_t long

//-----------------------------------------------------------------------------

static void* entc_alloc (number_t size)
{
  void* ptr = malloc (size);
  
  memset (ptr, 0, size);

  return ptr;
}

//-----------------------------------------------------------------------------

static void entc_free (void* ptr)
{
  free (ptr);
}

//-----------------------------------------------------------------------------

#define ENTC_ALLOC(size) entc_alloc(size)
#define ENTC_FREE(ptr) entc_free(ptr)

//-----------------------------------------------------------------------------

#define ENTC_NEW( type ) (type*)malloc(sizeof(type))
#define ENTC_DEL( ptr, type ) { memset(*ptr, 0, sizeof(type)); free(*ptr); *ptr = 0; }

//-----------------------------------------------------------------------------

#define ENTC_DIRECTION_FORW 0x0001
#define ENTC_DIRECTION_PREV 0x0002

#endif
