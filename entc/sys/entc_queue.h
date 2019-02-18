/*
 Copyright (c) 2019 Alexander Kalkhof
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#ifndef __ENTC_SYS__QUEUE__H
#define __ENTC_SYS__QUEUE__H 1

#include "sys/entc_export.h"
#include "sys/entc_err.h"

//=============================================================================

struct EntcQueue_s; typedef struct EntcQueue_s* EntcQueue;

typedef void (__STDCALL *entc_queue_cb_fct)(void* ptr);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcQueue         entc_queue_new         (void);           // allocate memory and initialize the object

__ENTC_LIBEX   void              entc_queue_del         (EntcQueue*);     // release memory

//-----------------------------------------------------------------------------

__ENTC_LIBEX   int               entc_queue_background  (EntcQueue, int amount_of_threads, EntcErr err);

__ENTC_LIBEX   void              entc_queue_add         (EntcQueue, entc_queue_cb_fct on_event, entc_queue_cb_fct on_done, void* ptr);

__ENTC_LIBEX   int               entc_queue_next        (EntcQueue);

//-----------------------------------------------------------------------------

#endif

