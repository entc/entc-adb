/*
 * Copyright (c) 2010-2018 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ADBO_ADBO_H
#define ADBO_ADBO_H 1

//=============================================================================

#include <system/ecdefs.h>
#include <types/ecerr.h>
#include <types/ecudc.h>

//-----------------------------------------------------------------------------

struct Adbo2Transaction_s; typedef struct Adbo2Transaction_s* Adbo2Transaction;

//-----------------------------------------------------------------------------

__LIBEX void adbo2_trx_commit     (Adbo2Transaction*);

__LIBEX void adbo2_trx_rollback   (Adbo2Transaction*);

__LIBEX int adbo2_trx_query       (Adbo2Transaction, const EcString table, EcUdc params, EcUdc data, EcErr);

__LIBEX int adbo2_trx_update      (Adbo2Transaction, const EcString table, EcUdc params, EcUdc data, EcErr);

__LIBEX int adbo2_trx_insert      (Adbo2Transaction, const EcString table, EcUdc params, EcUdc data, EcErr);

__LIBEX int adbo2_trx_delete      (Adbo2Transaction, const EcString table, EcUdc params, EcErr);

//-----------------------------------------------------------------------------

struct Adbo2_s; typedef struct Adbo2_s* Adbo2;

//-----------------------------------------------------------------------------

__LIBEX Adbo2 adbo2_create (const EcString confPath, const EcString binPath);

__LIBEX void adbo2_destroy (Adbo2*);

__LIBEX int adbo2_init (Adbo2, const EcString jsonConf);

__LIBEX Adbo2Transaction adbo2_transaction (Adbo2);

//-----------------------------------------------------------------------------

#endif
