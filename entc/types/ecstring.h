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

#include "../system/macros.h"
#include "../system/types.h"

#include "../types/eclist.h"
#include <time.h>

#define EcString char*

__CPP_EXTERN______________________________________________________________________________START

  /* **** basic methods **** */
  
__LIB_EXPORT EcString ecstr_init();

__LIB_EXPORT int ecstr_valid( const EcString );
  /* create a new string */
__LIB_EXPORT EcString ecstr_copy( const EcString );
  /* create a new string from pos 0 to length */
__LIB_EXPORT EcString ecstr_part( const EcString, uint_t length );
  
__LIB_EXPORT EcString ecstr_long( uint_t value );

__LIB_EXPORT EcString ecstr_float (double, uint_t n);
  
__LIB_EXPORT void ecstr_delete( EcString* );

__LIB_EXPORT EcString ecstr_replace( EcString*, const EcString );

__LIB_EXPORT EcString ecstr_replaceTO( EcString*, EcString );
  
__LIB_EXPORT EcString ecstr_replacePos( EcString*, const EcString, const EcString pos ); 
  
  /* **** merge methods **** */
  
__LIB_EXPORT EcString ecstr_cat2(const EcString, const EcString);

__LIB_EXPORT EcString ecstr_catc(const EcString, char c, const EcString);

__LIB_EXPORT EcString ecstr_cat3(const EcString, const EcString, const EcString);

__LIB_EXPORT EcString ecstr_cat4(const EcString, const EcString, const EcString, const EcString);
  
  /* **** const methods **** */
  
__LIB_EXPORT uint_t ecstr_len( const EcString );

__LIB_EXPORT const char* ecstr_cstring( const EcString );
  
__LIB_EXPORT int ecstr_equal( const EcString, const EcString );

__LIB_EXPORT int ecstr_equaln( const EcString, const EcString, uint_t size );
  
__LIB_EXPORT int ecstr_leading( const EcString, const EcString leading);

__LIB_EXPORT int ecstr_has (const EcString, char c);

__LIB_EXPORT const EcString ecstr_pos (const EcString, char c);

__LIB_EXPORT const EcString ecstr_spos (const EcString, char c, EcString* part);

__LIB_EXPORT const EcString ecstr_npos (const EcString, char c, uint_t max);

__LIB_EXPORT EcString ecstr_extractf( const EcString source, char c );

__LIB_EXPORT EcString ecstr_shrink (const EcString source, char from, char to);

__LIB_EXPORT int ecstr_split (const EcString source, EcString*, EcString*, char c);

  /* **** transform methods **** */
  
__LIB_EXPORT EcString ecstr_trim( const EcString );
  
__LIB_EXPORT EcString ecstr_trimNonePrintable( const EcString );
  
__LIB_EXPORT EcString ecstr_trimKeepDefault( const EcString );
  
__LIB_EXPORT EcString ecstr_trimEndLine( const EcString );

__LIB_EXPORT EcString ecstr_toVersion( uint_t version );

__LIB_EXPORT EcString ecstr_wrappedl (const EcString, char c);

// transform: no change on size

__LIB_EXPORT void ecstr_replaceAllChars( EcString, char find, char replace );

__LIB_EXPORT void ecstr_toLower(EcString);

__LIB_EXPORT void ecstr_toUpper(EcString);

  /* **** misc methods **** */
    
__LIB_EXPORT void ecstr_tokenizer(EcList, const EcString source, char delimeter);
  
__LIB_EXPORT void ecstr_tokenizer_clear(EcList);

__LIB_EXPORT EcString ecstr_tokenizer_get (EcList, EcListNode);
  
__LIB_EXPORT int ecstr_empty(const EcString);

__LIB_EXPORT EcString ecstr_extractParameter(char pn, int argc, char *argv[]);

__CPP_EXTERN______________________________________________________________________________END

#endif
