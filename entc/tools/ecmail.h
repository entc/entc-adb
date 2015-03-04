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

#ifndef ENTC_TOOLS_MAIL_H
#define ENTC_TOOLS_MAIL_H 1

/* include external macro for win32 */
#include "../system/macros.h"
#include "../system/ecevents.h"

#include "../types/ecstring.h"
#include "../types/ecstream.h"

#include "../utils/eclogger.h"

struct EcMail_s; typedef struct EcMail_s* EcMail;

__CPP_EXTERN______________________________________________________________________________START

__LIB_EXPORT EcMail ecmail_create (const EcString mailhost, uint_t port);
  
__LIB_EXPORT void ecmail_destroy (EcMail*);
  
__LIB_EXPORT int ecmail_send (EcMail, const EcString txtto, const EcString textre, const EcString subject, const EcString text, EcEventContext);

__CPP_EXTERN______________________________________________________________________________END

#endif
