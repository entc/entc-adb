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

#ifdef __DOS__

#include "qcmutex.h"
#include "../qcdefs.h"

#include <task.h>

//-----------------------------------------------------------------------------------

struct QCMutex_s {
  
  // will be implemented some day in the future
  
  int dummy;
  
};

//-----------------------------------------------------------------------------------

QCMutex qcmutex_new()
{
  QCMutex self = QNEW(struct QCMutex_s);
  
  return self;
}

//-----------------------------------------------------------------------------------

void qcmutex_delete(QCMutex* pself)
{
  QCMutex self = *pself;
    
  QDEL(pself, struct QCMutex_s);
}

//-----------------------------------------------------------------------------------

void qcmutex_lock(QCMutex self)
{
  taskENTER_CRITICAL();
}

//-----------------------------------------------------------------------------------

void qcmutex_unlock(QCMutex self)
{
  taskEXIT_CRITICAL();
}

//-----------------------------------------------------------------------------------

#endif
