/*
 * Copyright (c) 2010-2017 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_TESTS_ENV_H
#define ENTC_TESTS_ENV_H 1

//=============================================================================

#include "system/ecdefs.h"
#include "types/ecstring.h"
#include "types/ecerr.h"

//-----------------------------------------------------------------------------

struct TestEnv_s; typedef struct TestEnv_s* TestEnv;

struct TestEnvContext_s; typedef struct TestEnvContext_s* TestEnvContext;

typedef void* (__STDCALL *fct_testenv_init) (EcErr err);

typedef void (__STDCALL *fct_testenv_done) (void* ptr);

typedef int (__STDCALL *fct_testenv_test) (void* ptr, TestEnvContext ctx, EcErr err);

//-----------------------------------------------------------------------------

__LIBEX TestEnv testenv_create ();

__LIBEX int testenv_destroy (TestEnv*);

__LIBEX void testenv_run (TestEnv);

__LIBEX void testenv_reg (TestEnv, const char* name, fct_testenv_init, fct_testenv_done, fct_testenv_test);

//-----------------------------------------------------------------------------

__LIBEX void testctx_push_string (TestEnvContext, const char* text);

__LIBEX int testctx_pop_tocomp (TestEnvContext, const char* text);

__LIBEX void testctx_assert (TestEnvContext, int, const char* comment);

__LIBEX int testctx_err (TestEnvContext, EcErr);

//-----------------------------------------------------------------------------

#endif
