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

#ifndef ENTC_TOOLS_TOKENIZER_H
#define ENTC_TOOLS_TOKENIZER_H 1

//=============================================================================

#include "sys/entc_export.h"
#include "types/ecstring.h"
#include "stc/entc_list.h"

//-----------------------------------------------------------------------------

__ENTC_LIBEX EntcList ectokenizer_parse (const EcString source, char delimeter);           // tokenize using a character

__ENTC_LIBEX EntcList ectokenizer_token (const EcString source, const EcString token);     // tokenize using a token

//-----------------------------------------------------------------------------

#endif
