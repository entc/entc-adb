/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ecchain.h"
#include "eclist.h"

#include <stdio.h>

struct EcChain_s
{
  
  void** buffer;
  
  uint_t length;
  
  uint_t nextempty;
  
  EntcList stack;
  
  uint_t stacklen;
  
};

//----------------------------------------------------------------------------------------

static int __STDCALL ecchain_onDestroy (void* ptr)
{
  uint_t* pindex = ptr;
  
  ENTC_DEL (&pindex, uint_t);

  return 0;
}

//----------------------------------------------------------------------------------------

EcChain ecchain_create (EcAlloc alloc)
{
  EcChain self = ENTC_NEW(struct EcChain_s);
  
  self->stack = entc_list_new (ecchain_onDestroy);
  self->length = 200;
  self->buffer = ENTC_MALLOC(sizeof(void*) * self->length);
  
  ecchain_clear(self);
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecchain_clear(EcChain self)
{
  memset(self->buffer, 0x00, sizeof(void*) * self->length);
  self->nextempty = 0;
  
  entc_list_clr(self->stack);
  self->stacklen = 0;
}

//----------------------------------------------------------------------------------------

void ecchain_destroy (EcAlloc alloc, EcChain* pself)
{
  EcChain self = *pself;
  
  ecchain_clear (self);
  
  entc_list_del (&(self->stack));
  
  ENTC_FREE(self->buffer);
  
  ENTC_DEL(pself, struct EcChain_s);
}

//----------------------------------------------------------------------------------------

uint_t ecchain_getNextIndex(EcChain self)
{
  uint_t res;
  
  if (self->stacklen > 0) 
  {
    uint_t* pindex;
    
    self->stacklen--;
    
    pindex = eclist_pop_front (self->stack);
    
    res = *pindex;
    
    ENTC_DEL(&pindex, uint_t);
  }
  else
  {
    res = self->nextempty;
    if (self->nextempty == self->length)
    {
      uint_t len;
      // increase by 2 x length
      printf("increase by 100%%\n");
      
      len = self->length;
      
      self->length = len + len;      
      self->buffer = realloc (self->buffer, self->length * sizeof(void*));
      
      memset(self->buffer + len, 0x00, len * sizeof(void*));      
    }
    self->nextempty++;
  }
  return res;
}

//----------------------------------------------------------------------------------------

uint_t ecchain_add(EcChain self, void* data)
{
  uint_t pos = ecchain_getNextIndex(self);
  
  void** pp = self->buffer + pos;
  
  *pp = data;
  
  return pos;
}

//----------------------------------------------------------------------------------------

void ecchain_del(EcChain self, uint_t index)
{
  // variables
  uint_t* pindex;
  void** pp;

  if (index >= self->nextempty) 
  {
    return;
  }
  
  pindex = ENTC_NEW(uint_t);  
  *pindex = index;
  entc_list_push_back (self->stack, pindex);
  self->stacklen++;
  
  pp = self->buffer + index;
  *pp = NULL;
}

//----------------------------------------------------------------------------------------

void* ecchain_get(EcChain self, uint_t index)
{
  if (index < self->length)
  {
    void** pp = self->buffer + index;
    return *pp;
  }
  return NULL;
}

//----------------------------------------------------------------------------------------

int ecchain_isValidIndex(const EcChain self, uint_t index)
{
  // variables
  void** pp;

  if (index >= self->nextempty) 
  {
    return -1;
  }
  
  pp = self->buffer + index;
  return *pp != NULL;
}

//----------------------------------------------------------------------------------------

uint_t ecchain_begin(const EcChain self)
{
  uint_t i = 0;
  int res = FALSE;
  
  do
  {
    res = ecchain_isValidIndex(self, i);
    if (res < 0) 
    {
      return ecchain_end(self);
    }
    i++;
  }
  while (!res);
  
  return i - 1;
}

//----------------------------------------------------------------------------------------

uint_t ecchain_end(const EcChain self)
{
  return self->nextempty;
}

//----------------------------------------------------------------------------------------

uint_t ecchain_next(const EcChain self, uint_t index)
{
  uint_t i = index + 1;
  int res = FALSE;
  
  do
  {
    res = ecchain_isValidIndex(self, i);
    if (res < 0) 
    {
      return ecchain_end(self);
    }
    i++;
  }
  while (!res);
  
  return i - 1;
}

//----------------------------------------------------------------------------------------

/*
void printInfo(const EcChain self)
{
  printf("[%p] length %u, free (%u, %u)\n", self, self->length, entc_list_size(self->stack), self->nextempty);
}

//----------------------------------------------------------------------------------------

void ecchain_dumpStack(const EcChain self)
{
  uint_t i = 0;
  EntcListNode node;

  printf("start dump of list [%p] length %u %u\n", self, self->stacklen, entc_list_size(self->stack));
  
  if (self->stacklen != entc_list_size(self->stack))
  {
    printf("stack length mismatch\n");
  }
 
  for (node = eclist_first(self->stack); node != eclist_end(self->stack); node = eclist_next(node), i++)
  {
    uint_t* pindex = entc_list_node_data(node);
    
    printf("[%u %p]: %u | ", i, pindex, *pindex);
  }
  printf("\n");
}

//----------------------------------------------------------------------------------------

void ecchain_dumpArray(const EcChain self)
{
  uint_t i;
  
  printf("start dump of array [%p] length %u %u\n", self, self->length, self->nextempty);

  for (i = 0; i < self->length; i++)
  {
    void** pp = self->buffer + i;
    
    printf("[%u %p]", i, *pp);
  }
  printf("\n");
}
 */

//----------------------------------------------------------------------------------------
