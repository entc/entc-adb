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


