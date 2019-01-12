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

#ifndef __ENTC_STC__UDC__H
#define __ENTC_STC__UDC__H 1

#include "sys/entc_export.h"
#include "stc/entc_str.h"

//-----------------------------------------------------------------------------

#define ENTC_UDC_NODE         1
#define ENTC_UDC_LIST         2
#define ENTC_UDC_STRING       3
#define ENTC_UDC_NUMBER       4
#define ENTC_UDC_FLOAT        5
#define ENTC_UDC_BOOL         6

//=============================================================================

struct EntcUdc_s; typedef struct EntcUdc_s* EntcUdc;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcUdc           entc_udc_new              (u_t type, const EntcString name);

__ENTC_LIBEX   void              entc_udc_del              (EntcUdc*);

__ENTC_LIBEX   EntcUdc           entc_udc_cp               (const EntcUdc);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   const EntcString  entc_udc_name             (const EntcUdc);

__ENTC_LIBEX   u_t               entc_udc_type             (const EntcUdc);

__ENTC_LIBEX   void*             entc_udc_data             (const EntcUdc);   // returns the pointer of type address

__ENTC_LIBEX   number_t          entc_udc_size             (const EntcUdc);

__ENTC_LIBEX   void              entc_udc_set_name         (const EntcUdc, const EntcString name);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcUdc           entc_udc_add              (EntcUdc, EntcUdc*);

__ENTC_LIBEX   EntcUdc           entc_udc_add_name         (EntcUdc, EntcUdc*, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_udc_get              (EntcUdc, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_udc_ext              (EntcUdc, const EntcString name);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void              entc_udc_set_s_cp         (EntcUdc, const EntcString val);

__ENTC_LIBEX   void              entc_udc_set_s_mv         (EntcUdc, EntcString* p_val);

__ENTC_LIBEX   void              entc_udc_set_n            (EntcUdc, number_t val);

__ENTC_LIBEX   void              entc_udc_set_f            (EntcUdc, double val);

__ENTC_LIBEX   void              entc_udc_set_b            (EntcUdc, int val);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   const EntcString  entc_udc_s                (EntcUdc, const EntcString alt);

__ENTC_LIBEX   EntcString        entc_udc_s_mv             (EntcUdc, const EntcString alt);

__ENTC_LIBEX   number_t          entc_udc_n                (EntcUdc, number_t alt);

__ENTC_LIBEX   double            entc_udc_f                (EntcUdc, double alt);

__ENTC_LIBEX   int               entc_udc_b                (EntcUdc, int alt);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcUdc           entc_udc_add_s_cp         (EntcUdc, const EntcString name, const EntcString val);

__ENTC_LIBEX   EntcUdc           entc_udc_add_s_mv         (EntcUdc, const EntcString name, EntcString* p_val);

__ENTC_LIBEX   EntcUdc           entc_udc_add_n            (EntcUdc, const EntcString name, number_t val);

__ENTC_LIBEX   EntcUdc           entc_udc_add_f            (EntcUdc, const EntcString name, double val);

__ENTC_LIBEX   EntcUdc           entc_udc_add_b            (EntcUdc, const EntcString name, int val);

__ENTC_LIBEX   EntcUdc           entc_udc_add_node         (EntcUdc, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_udc_add_list         (EntcUdc, const EntcString name);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   const EntcString  entc_udc_get_s            (EntcUdc, const EntcString name, const EntcString alt);

__ENTC_LIBEX   number_t          entc_udc_get_n            (EntcUdc, const EntcString name, number_t alt);

__ENTC_LIBEX   double            entc_udc_get_f            (EntcUdc, const EntcString name, double alt);

__ENTC_LIBEX   int               entc_udc_get_b            (EntcUdc, const EntcString name, int alt);

__ENTC_LIBEX   EntcUdc           entc_udc_get_node         (EntcUdc, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_udc_get_list         (EntcUdc, const EntcString name);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcString        entc_udc_ext_s            (EntcUdc, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_udc_ext_node         (EntcUdc, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_udc_ext_list         (EntcUdc, const EntcString name);

//-----------------------------------------------------------------------------

typedef struct
{
  
  EntcUdc item;
  
  int position;
  
  int direction;
  
  void* data;   // for internal use (don't change it)
  u_t type;     // for internal use (don't change it)
  
} EntcUdcCursor;

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcUdcCursor*    entc_udc_cursor_new       (EntcUdc, int direction);

__ENTC_LIBEX   void              entc_udc_cursor_del       (EntcUdcCursor**);

__ENTC_LIBEX   int               entc_udc_cursor_next      (EntcUdcCursor*);

__ENTC_LIBEX   int               entc_udc_cursor_prev      (EntcUdcCursor*);

__ENTC_LIBEX   void              entc_udc_cursor_rm        (EntcUdc, EntcUdcCursor*);

__ENTC_LIBEX   EntcUdc           entc_udc_cursor_ext       (EntcUdc, EntcUdcCursor*);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   void              entc_udc_print            (const EntcUdc);

//=============================================================================

#endif
