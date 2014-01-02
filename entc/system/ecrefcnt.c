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

#include "ecrefcnt.h"

struct EcRefCnt_s
{
  
  int cnt;
  
  void* data;
  
};

//-----------------------------------------------------------------------------------

EcRefCnt ecrefcnt_new (void* data)
{
  EcRefCnt self = ENTC_NEW (struct EcRefCnt_s);
  
  self->cnt = 1;
  self->data = data;
  
  return self;  
}

//-----------------------------------------------------------------------------------

void ecrefcnt_delete (EcRefCnt* pself)
{
  ENTC_DEL (pself, struct EcRefCnt_s);  
}

//-----------------------------------------------------------------------------------

void ecrefcnt_inc (EcRefCnt self)
{
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
  __sync_add_and_fetch(&(self->cnt), 1);
#else
  self->cnt++;
#endif  
}

//-----------------------------------------------------------------------------------

int ecrefcnt_dec (EcRefCnt self)
{
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
  return (__sync_sub_and_fetch(&(self->cnt), 1);
#else
  self->cnt--;
  return self->cnt;
#endif    
}

//-----------------------------------------------------------------------------------

void* ecrefcnt_get (EcRefCnt self)
{
  return self->data;
}

//-----------------------------------------------------------------------------------


