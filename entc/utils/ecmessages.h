/*
 * Copyright (c) 2010-2015 "Alexander Kalkhof" [email:entc@kalkhof.org]
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

#ifndef ENTC_UTILS_MESSAGES_H
#define ENTC_UTILS_MESSAGES_H 1

#include <system/macros.h>
#include <system/types.h>

#include <types/eclist.h>
#include <types/ecstring.h>
#include <types/ecudc.h>


struct EcMessages_s; typedef struct EcMessages_s* EcMessages;

struct EcMessagesOutput_s; typedef struct EcMessagesOutput_s* EcMessagesOutput;

//---------------------------------------------------------------------------------------------

#pragma pack(push, 1)
typedef struct {
  
  uint_t type;
  uint_t rev;
  
  uint_t ref;  
  
  EcUdc content;
  
} EcMessageData;
#pragma pack(pop)

// function definitions
typedef int (_STDCALL *ecmessages_request_fct) (void* ptr, EcMessageData* dIn, EcMessageData* dOut);
typedef int (_STDCALL *ecmessages_result_fct) (void* ptr, EcMessageData* data, int errorcode);

// this are reserved constants from the system
#define ENTC_MSGMODD_LOG 100
#define ENTC_MSGSRVC_LOG 100
#define ENTC_MSGTYPE_LOG 100

__CPP_EXTERN______________________________________________________________________________START

// creates a global instance of the messaging system
__LIB_EXPORT void ecmessages_initialize ();

// deletes the global instance
__LIB_EXPORT void ecmessages_deinitialize ();

__LIB_EXPORT void ecmessages_add (uint_t module, uint_t method, ecmessages_request_fct, void* ptr);

__LIB_EXPORT void ecmessages_removeAll (uint_t module);

// broadcast output

__LIB_EXPORT EcMessagesOutput ecmessages_output_create (ecmessages_result_fct, void* ptr, uint_t type, uint_t rev);

__LIB_EXPORT void ecmessages_output_destroy (EcMessagesOutput*);

// send messages

__LIB_EXPORT int ecmessages_broadcast (uint_t method, EcMessageData* data, EcMessagesOutput output);

__LIB_EXPORT int ecmessages_send (uint_t module, uint_t method, EcMessageData* dIn, EcMessageData* dOut);

// misc

__LIB_EXPORT void ecmessages_initData (EcMessageData*, uint_t type, uint_t rev);

__LIB_EXPORT void ecmessages_initDataN (EcMessageData*, uint_t type, uint_t rev, uint_t ref, const EcString nodeName);

__LIB_EXPORT void ecmessages_clearData (EcMessageData*);

__LIB_EXPORT void ecmessages_resetData_KOCN (EcMessageData*, uint_t type, uint_t rev, uint_t ref, const EcString nodeName);

__CPP_EXTERN______________________________________________________________________________END

#endif
