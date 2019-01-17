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

#include "sys/entc_export.h"
#include "sys/entc_types.h"

struct EcTable_s; typedef struct EcTable_s* EcTable;

typedef uint_t EcTableNode;

__ENTC_LIBEX EcTable ectable_new(uint_t cols, uint_t rows);
    
__ENTC_LIBEX void ectable_delete( EcTable* );
  
__ENTC_LIBEX void ectable_clear( EcTable );
  
__ENTC_LIBEX void ectable_copy( EcTable, EcTable );
  
__ENTC_LIBEX void ectable_set( EcTable, uint_t row, uint_t col, void* value);
  
__ENTC_LIBEX void* ectable_get( EcTable, uint_t row, uint_t col );
  
  /* iterators */  
  
__ENTC_LIBEX EcTableNode ectable_first( EcTable );
  
__ENTC_LIBEX EcTableNode ectable_end( EcTable );
  
__ENTC_LIBEX EcTableNode ectable_next( EcTable, EcTableNode );
  
__ENTC_LIBEX void* ectable_data( EcTable, EcTableNode, uint_t col );
  
  /* getters */
  
__ENTC_LIBEX uint_t ectable_getColumns( EcTable );

__ENTC_LIBEX uint_t ectable_getRows( EcTable );

__ENTC_LIBEX void ectable_echo( EcTable );
  
#endif
