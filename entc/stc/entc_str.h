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

#ifndef __ENTC_STC__STR__H
#define __ENTC_STC__STR__H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"

//=============================================================================

#define EntcString char*

__ENTC_LIBEX   EntcString         entc_str_cp            (const EntcString);                          // allocate memory and initialize the object

__ENTC_LIBEX   void               entc_str_del           (EntcString*);                               // release memory

__ENTC_LIBEX   EntcString         entc_str_sub           (const EntcString, number_t len);            // copy a part of the substring

__ENTC_LIBEX   EntcString         entc_str_uuid          (void);                                      // create an UUID and copy it into the string

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcString         entc_str_catenate_c    (const EntcString, char c, const EntcString);

__ENTC_LIBEX   EntcString         entc_str_catenate_2    (const EntcString, const EntcString);

__ENTC_LIBEX   EntcString         entc_str_catenate_3    (const EntcString, const EntcString, const EntcString);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void               entc_str_replace_cp    (EntcString*, const EntcString source);      // replaces the object with a const string

__ENTC_LIBEX   void               entc_str_replace_mv    (EntcString*, EntcString*);                  // replaces the object with another object
  
//-----------------------------------------------------------------------------

__ENTC_LIBEX   void               entc_str_fill          (EntcString, number_t len, const EntcString source);       // will cut the content if not enough memory

__ENTC_LIBEX   void               entc_str_to_upper      (EntcString);

__ENTC_LIBEX   void               entc_str_to_lower      (EntcString);

//-----------------------------------------------------------------------------

#endif


