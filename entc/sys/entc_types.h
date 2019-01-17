#ifndef __ENTC_SYS__TYPES__H
#define __ENTC_SYS__TYPES__H 1

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

//-----------------------------------------------------------------------------

#define TRUE 1
#define FALSE 0

#define u_t unsigned
#define number_t long

// depricated defines
typedef unsigned char ubyte_t;
typedef char byte_t;

typedef unsigned long ulong_t;
typedef long long_t;

typedef unsigned int uint_t;
typedef signed int int_t;

#define isAssigned(pointer) (pointer != NULL)
#define isNotAssigned(pointer) (pointer == NULL)

#define ENTC_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ENTC_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define ENTC_INFINITE           1000000

//-----------------------------------------------------------------------------

#define ENTC_MALLOC(size) malloc(size)
#define ENTC_FREE(ptr) free(ptr)

//-----------------------------------------------------------------------------

#define ENTC_NEW( type ) (type*)malloc(sizeof(type))
#define ENTC_DEL( ptr, type ) { memset(*ptr, 0, sizeof(type)); free(*ptr); *ptr = 0; }

//-----------------------------------------------------------------------------

#define ENTC_DIRECTION_FORW 0x0001
#define ENTC_DIRECTION_PREV 0x0002

#endif
