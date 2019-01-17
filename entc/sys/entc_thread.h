/*
 C opyright (c) 2019 Alexander *Kalkhof
 
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

#ifndef __ENTC_SYS__THREAD__H
#define __ENTC_SYS__THREAD__H 1

//-----------------------------------------------------------------------------

#include "sys/entc_export.h"

//-----------------------------------------------------------------------------

typedef int (__STDCALL *entc_thread_worker_fct)(void* ptr);

struct EntcThread_s; typedef struct EntcThread_s* EntcThread;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcThread        entc_thread_new        (void);                // allocate memory and initialize the object

__ENTC_LIBEX   void              entc_thread_del        (EntcThread*);

__ENTC_LIBEX   void              entc_thread_start      (EntcThread, entc_thread_worker_fct, void* ptr);

__ENTC_LIBEX   void              entc_thread_join       (EntcThread);

__ENTC_LIBEX   void              entc_thread_cancel     (EntcThread);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void              entc_thread_sleep      (unsigned long milliseconds);

#endif
