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

#include "ecnotify.h"

struct EcNotify_s
{
    void* dummy;
    
};

//------------------------------------------------------------------------------------------------------

EcNotify ecnotify_create ()
{
    EcNotify self = ENTC_NEW (struct EcNotify_s);
    
    
    return self;
}

//------------------------------------------------------------------------------------------------------

void ecnotify_destroy (EcNotify* pself)
{
    EcNotify self = *pself;

    ENTC_DEL(pself, struct EcNotify_s);
}

//------------------------------------------------------------------------------------------------------

void enotify_addPath (EcNotify self, const EcString path)
{
    
}

//------------------------------------------------------------------------------------------------------
