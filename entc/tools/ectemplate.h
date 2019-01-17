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

#ifndef ENTC_TOOLS_TEMPLATE_H
#define ENTC_TOOLS_TEMPLATE_H 1

//-----------------------------------------------------------------------------

// entc includes
#include <types/ecerr.h>
#include "types/ecstring.h"
#include "types/ecudc.h"

//=============================================================================

struct EcTemplate_s; typedef struct EcTemplate_s* EcTemplate;

//-----------------------------------------------------------------------------

typedef int (__STDCALL *fct_ectemplate_onText) (void* ptr, const char* text);
typedef int (__STDCALL *fct_ectemplate_onFile) (void* ptr, const char* file);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EcTemplate    ectemplate_create         (void);

__ENTC_LIBEX   void          ectemplate_destroy        (EcTemplate*);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   int           ectemplate_compile_file   (EcTemplate, const char* path, const char* name, const char* lang, EcErr);

__ENTC_LIBEX   int           ectemplate_compile_str    (EcTemplate, const char* content, EcErr);

__ENTC_LIBEX   int           ectemplate_apply          (EcTemplate, EcUdc node, void* ptr, fct_ectemplate_onText, fct_ectemplate_onFile, EcErr);

//-----------------------------------------------------------------------------

#endif
