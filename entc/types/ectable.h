/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:alex@kalkhof.org]
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

#ifndef ENTC_TYPES_TABLE_H
#define ENTC_TYPES_TABLE_H 1

/* include external macro for win32 */
#include "../system/macros.h"
#include "../system/types.h"

struct EcTable_s; typedef struct EcTable_s* EcTable;

typedef uint_t EcTableNode;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcTable ectable_new(uint_t cols, uint_t rows);
    
__LIB_EXPORT void ectable_delete( EcTable* );
  
__LIB_EXPORT void ectable_clear( EcTable );
  
__LIB_EXPORT void ectable_copy( EcTable, EcTable );
  
__LIB_EXPORT void ectable_set( EcTable, uint_t row, uint_t col, void* value);
  
__LIB_EXPORT void* ectable_get( EcTable, uint_t row, uint_t col );
  
  /* iterators */  
  
__LIB_EXPORT EcTableNode ectable_first( EcTable );
  
__LIB_EXPORT EcTableNode ectable_end( EcTable );
  
__LIB_EXPORT EcTableNode ectable_next( EcTable, EcTableNode );
  
__LIB_EXPORT void* ectable_data( EcTable, EcTableNode, uint_t col );
  
  /* getters */
  
__LIB_EXPORT uint_t ectable_getColumns( EcTable );

__LIB_EXPORT uint_t ectable_getRows( EcTable );

__LIB_EXPORT void ectable_echo( EcTable );
  
__CPP_EXTERN______________________________________________________________________________END

#endif
