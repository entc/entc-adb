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

#ifndef ENTC_TYPES_STRING_H
#define ENTC_TYPES_STRING_H 1

//-----------------------------------------------------------------------------

#include "system/ecdefs.h"
#include "system/types.h"

#include <time.h>

#ifdef __GNUC__
#include <stdarg.h>
#endif

//-----------------------------------------------------------------------------

#define EcString char*

//=============================================================================

__LIBEX EcString ecstr_init();                 // create not valid string

__LIBEX int ecstr_valid (const EcString);

__LIBEX int ecstr_empty (const EcString);

__LIBEX EcString ecstr_copy (const EcString);  // copy string

__LIBEX EcString ecstr_part (const EcString, uint_t length);  // create a new string from pos 0 to length
  
__LIBEX EcString ecstr_long (uint64_t value);

__LIBEX EcString ecstr_longPadded (uint64_t value, int amount);

__LIBEX EcString ecstr_float (double, uint_t n);

__LIBEX EcString ecstr_filled (char c, uint_t n);

__LIBEX EcString ecstr_create_fmt (uint_t size, const EcString format, ...);

__LIBEX EcString ecstr_format (const EcString format, ...);

__LIBEX EcString ecstr_format_list (const EcString format, va_list ptr);
  
__LIBEX void ecstr_delete( EcString* );

__LIBEX EcString ecstr_replace( EcString*, const EcString );

__LIBEX EcString ecstr_replaceTO( EcString*, EcString );
  
__LIBEX EcString ecstr_replacePos( EcString*, const EcString, const EcString pos ); 
  
//-----------------------------------------------------------------------------

  /* **** merge methods **** */
  
__LIBEX EcString ecstr_cat2 (const EcString, const EcString);

__LIBEX EcString ecstr_catc (const EcString, char c, const EcString);

__LIBEX EcString ecstr_cat3 (const EcString, const EcString, const EcString);

__LIBEX EcString ecstr_cat4 (const EcString, const EcString, const EcString, const EcString);
  
//-----------------------------------------------------------------------------

  /* **** const methods **** */
  
__LIBEX uint_t ecstr_len ( const EcString );

__LIBEX const char* ecstr_cstring ( const EcString );
  
__LIBEX int ecstr_equal( const EcString, const EcString );

__LIBEX int ecstr_equaln( const EcString, const EcString, uint_t size );

__LIBEX int ecstr_equalUnsensitive (const EcString, const EcString);
  
__LIBEX int ecstr_leading ( const EcString, const EcString leading);

__LIBEX int ecstr_ending ( const EcString, const EcString ending);

__LIBEX int ecstr_has (const EcString, char c);

__LIBEX const EcString ecstr_pos (const EcString, char c);

__LIBEX const EcString ecstr_spos (const EcString, char c, EcString* part);

__LIBEX const EcString ecstr_npos (const EcString, char c, uint_t max);

__LIBEX EcString ecstr_extractf( const EcString source, char c );

__LIBEX EcString ecstr_shrink (const EcString source, char from, char to);

__LIBEX int ecstr_split (const EcString source, EcString*, EcString*, char c);

__LIBEX int ecstr_leadingPart (const EcString source, const EcString leading, EcString*);

__LIBEX int ecstr_endingPart (const EcString source, const EcString ending, EcString*);

//-----------------------------------------------------------------------------

/* **** transform methods **** */
  
__LIBEX EcString ecstr_trim( const EcString );

__LIBEX EcString ecstr_trimFullAscii (const EcString);

__LIBEX EcString ecstr_trimNoneNumbers (const EcString);

__LIBEX EcString ecstr_trimNonePrintable( const EcString );
  
__LIBEX EcString ecstr_trimKeepDefault( const EcString );
  
__LIBEX EcString ecstr_trimEndLine( const EcString );

// trim the string only by a certain character
__LIBEX EcString ecstr_trimc (const EcString, char c);

// trim the string only by a certain character
__LIBEX EcString ecstr_trimlr (const EcString, char l, char r);

__LIBEX EcString ecstr_toVersion( uint_t version );

__LIBEX EcString ecstr_wrappedl (const EcString, char c);

// left and right can be NULL
__LIBEX EcString ecstr_unwrapl (const EcString, char cl, char cr, EcString* left, EcString* right);

__LIBEX EcString ecstr_lpad (const EcString, char c, uint_t len);

__LIBEX EcString ecstr_removeAllChars (const EcString, char find);

__LIBEX EcString ecstr_replaceS (const EcString, const EcString, const EcString);

// transform: no change on size

__LIBEX void ecstr_replaceAllChars( EcString, char find, char replace );

__LIBEX void ecstr_toLower(EcString);

__LIBEX void ecstr_toUpper(EcString);

#ifdef _WIN32

__LIBEX wchar_t* ecstr_utf8ToWide (const EcString);

#endif

//-----------------------------------------------------------------------------
 
#endif
