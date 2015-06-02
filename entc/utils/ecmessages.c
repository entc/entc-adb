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

#include "ecmessages.h"

// include system
#include "system/ecmutex.h"
#include "system/ecfile.h"

// include type
#include "types/ecintmap.h"

#include <stdio.h>

struct EcMessages_s
{
  
  EcReadWriteLock mutex;
  
  EcIntMap functions;
  
  // for logging
  
  EcFileHandle fh;
  
  EcMutex fhmutex;
  
};

struct EcMessagesOutput_s
{
  
  ecmessages_result_fct fct;
  
  void* ptr;
  
  uint_t type;
  
  uint_t rev;
  
};

typedef struct 
{
  
  ecmessages_request_fct fct;
  
  void* ptr;
  
} EcMessageModule;

static EcMessages getGlobalMessages (void)
{
  static struct EcMessages_s h;
  return &h; 
}

//----------------------------------------------------------------------------------------

static const char* msg_matrix[7] = { "___", "FAT", "ERR", "WRN", "INF", "DBG", "TRA" };
#if defined _WIN64 || defined _WIN32
#include <windows.h>
static WORD clr_matrix[11] = {
  0, 
  FOREGROUND_RED | FOREGROUND_INTENSITY,
  FOREGROUND_RED | FOREGROUND_INTENSITY,
  FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
  FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,
  FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED,
  FOREGROUND_GREEN | FOREGROUND_BLUE
};
#else
static const char* clr_matrix[7] = { "0", "0;31", "1;31", "1;33", "0;32", "0;34", "0;30" };
#endif

int _STDCALL ecmessages_logger_callback (void* ptr, EcMessageData* dIn, EcMessageData* dOut)
{
  if (isAssigned (dIn) && (dIn->type == ENTC_MSGTYPE_LOG) && (dIn->rev == 1))
  {
    if (dIn->ref < 7)
    {
      static char buffer [2050];
      EcMessages self = ptr;

      ecmutex_lock (self->fhmutex);
                 
#if defined _WIN64 || defined _WIN32 

      _snprintf_s (buffer, 2048, _TRUNCATE, "%-12s %s|%-6s] %s", ecudc_get_asString(dIn->content, "method", ""), msg_matrix[dIn->ref], ecudc_get_asString(dIn->content, "unit", "____"), ecudc_get_asString(dIn->content, "msg", ""));
      {
        CONSOLE_SCREEN_BUFFER_INFO info;
        // get the console handle
        HANDLE hStdout = GetStdHandle (STD_OUTPUT_HANDLE);      
        // remember the original background color
        GetConsoleScreenBufferInfo (hStdout, &info);
        // do some fancy stuff
        SetConsoleTextAttribute (hStdout, clr_matrix[dIn->ref]);

        printf("%s\n", buffer);

        SetConsoleTextAttribute (hStdout, info.wAttributes);
      }
#else
      snprintf (buffer, 2048, "%-12s %s|%-6s] %s", ecudc_get_asString(dIn->content, "method", ""), msg_matrix[dIn->ref], ecudc_get_asString(dIn->content, "unit", "____"), ecudc_get_asString(dIn->content, "msg", ""));

      printf("\033[%sm%s\033[0m\n", clr_matrix[dIn->ref], buffer);
#endif
      {
        if (self->fh)
        {
          ecfh_writeString (self->fh, buffer);
          ecfh_writeString (self->fh, "\n");
        }
      }
      
      ecmutex_unlock (self->fhmutex);
    }    
  }
  
  return ENTC_RESCODE_IGNORE;
}

//----------------------------------------------------------------------------------------

void ecmessages_initialize ()
{
  EcMessages self = getGlobalMessages ();

  self->mutex = ecreadwritelock_new ();
  self->functions = ecintmap_new ();
  
  {
    EcString currentDir = ecfs_getCurrentDirectory ();
    
    EcString defaultLogFile = ecfs_mergeToPath (currentDir, "out.log");
    
    self->fh = ecfh_open (defaultLogFile, O_WRONLY | O_APPEND | O_CREAT);
    self->fhmutex = ecmutex_new ();
    
    ecstr_delete (&defaultLogFile);
    ecstr_delete (&currentDir);
  }
  
  ecmessages_add (ENTC_MSGMODD_LOG, ENTC_MSGSRVC_LOG, ecmessages_logger_callback, self);
}

//----------------------------------------------------------------------------------------

void ecmessages_add_next (EcIntMap modules, uint_t module, ecmessages_request_fct fct, void* ptr)
{
  EcIntMapNode node = ecintmap_find (modules, module);
  if (node == ecintmap_end (modules))
  {
    EcMessageModule* item = ENTC_NEW (EcMessageModule);
    
    item->fct = fct;
    item->ptr = ptr;
    
    ecintmap_append (modules, module, item);
  }
}

//----------------------------------------------------------------------------------------

void ecmessages_add (uint_t module, uint_t method, ecmessages_request_fct fct, void* ptr)
{
  EcIntMap modules;
  EcIntMapNode node;

  EcMessages self = getGlobalMessages ();

  ecreadwritelock_lockWrite (self->mutex);
  
  node = ecintmap_find (self->functions, method);
  if (node == ecintmap_end (self->functions))
  {
    modules = ecintmap_new ();
    ecintmap_append (self->functions, method, modules);
  }
  else
  {
    modules = ecintmap_data (node);
  }

  ecmessages_add_next (modules, module, fct, ptr);

  ecreadwritelock_unlockWrite (self->mutex);
}

//----------------------------------------------------------------------------------------

void ecmessages_removeAll_next (EcIntMap modules, uint_t module)
{
  EcIntMapNode node = ecintmap_find (modules, module);
  if (node != ecintmap_end (modules))
  {
    EcMessageModule* item = ecintmap_data (node);
    
    ENTC_DEL (&item, EcMessageModule);
    
    ecintmap_erase (node);
  }
}

//----------------------------------------------------------------------------------------

void ecmessages_removeAll (uint_t module)
{
  EcMessages self = getGlobalMessages ();

  EcIntMapNode node;

  ecreadwritelock_lockWrite (self->mutex);

  for (node = ecintmap_first (self->functions); node != ecintmap_end (self->functions); node = ecintmap_next (node))
  {
    EcIntMap modules = ecintmap_data (node);
    
    ecmessages_removeAll_next (modules, module);
    
    if (ecintmap_first (modules) == ecintmap_end (modules))
    {
      // the modules is empty, remove it also from first map
      ecintmap_delete (&modules);
      node = ecintmap_erase (node);
    }
  }
  
  ecreadwritelock_unlockWrite (self->mutex);
}

//----------------------------------------------------------------------------------------

void ecmessages_clear_modules (EcIntMap modules)
{
  EcIntMapNode node;
  for (node = ecintmap_first (modules); node != ecintmap_end (modules); node = ecintmap_next (node))
  {
    EcMessageModule* item = ecintmap_data (node);
    
    ENTC_DEL (&item, EcMessageModule);
  }
}

//----------------------------------------------------------------------------------------

void ecmessages_clear ()
{
  EcMessages self = getGlobalMessages ();

  EcIntMapNode node;
  for (node = ecintmap_first (self->functions); node != ecintmap_end (self->functions); node = ecintmap_next (node))
  {
    EcIntMap modules = ecintmap_data (node);

    ecmessages_clear_modules (modules);
    
    ecintmap_delete (&modules);
  }
  
  ecintmap_clear (self->functions);
}

//----------------------------------------------------------------------------------------

void ecmessages_deinitialize ()
{
  EcMessages self = getGlobalMessages ();

  ecmessages_clear ();
  
  ecfh_close (&(self->fh));
  ecmutex_delete (&(self->fhmutex));
  
  ecintmap_delete (&(self->functions));
  ecreadwritelock_delete (&(self->mutex));
}

//----------------------------------------------------------------------------------------

EcMessagesOutput ecmessages_output_create (ecmessages_result_fct fct, void* ptr, uint_t type, uint_t rev)
{
  EcMessagesOutput self = ENTC_NEW (struct EcMessagesOutput_s);
  
  self->fct = fct;
  self->ptr = ptr;
  
  self->type = type;
  self->rev = rev;
  
  return self;
}

//----------------------------------------------------------------------------------------

void ecmessages_output_destroy (EcMessagesOutput* pself)
{
  ENTC_DEL (pself, struct EcMessagesOutput_s);
}

//----------------------------------------------------------------------------------------

int ecmessages_broadcast_next (EcIntMap modules, EcMessageData* data, EcMessagesOutput output)
{
  int ret = 0;
  
  EcIntMapNode node;
  for (node = ecintmap_first (modules); node != ecintmap_end (modules); node = ecintmap_next (node))
  {
    EcMessageModule* item = ecintmap_data (node);
    
    if (isAssigned (item->fct))
    {
      int errorcode;
      EcMessageData out;      
            
      if (isAssigned (output))
      {
        ecmessages_initData (&out, output->type, output->rev);
      }
      else
      {
        ecmessages_initData (&out, 0, 0);        
      }
      
      // call the callback function and maybe fill out with data
      errorcode = item->fct (item->ptr, data, &out);
      
      if (errorcode != ENTC_RESCODE_IGNORE)
      {
        // if a output callback was set, this is called with the out data as input
        if (isAssigned (output) && isAssigned (output->fct))
        {
          if (output->fct (output->ptr, &out, errorcode) == ENTC_RESCODE_OK)
          {
            ret++;
          }
        }
        else
        {
          ret++;
        }
      }
      
      if (isAssigned (out.content))
      {
        ecudc_destroy(&(out.content));
      }
    }
  }
  
  return ret;  
}

//----------------------------------------------------------------------------------------

int ecmessages_broadcast (uint_t method, EcMessageData* data, EcMessagesOutput output)
{
  EcMessages self = getGlobalMessages ();

  int ret = ENTC_RESCODE_IGNORE;
  EcIntMapNode node;

  ecreadwritelock_lockRead (self->mutex);

  node = ecintmap_find (self->functions, method);
  if (node != ecintmap_end (self->functions))
  {
    ret = ecmessages_broadcast_next (ecintmap_data (node), data, output);
  }

  ecreadwritelock_unlockRead (self->mutex);
  
  return ret;
}

//----------------------------------------------------------------------------------------

int ecmessages_send_next (EcIntMap modules, uint_t module, EcMessageData* dIn, EcMessageData* dOut)
{
  int ret = ENTC_RESCODE_NOT_FOUND;
  
  EcIntMapNode node = ecintmap_find (modules, module);
  if (node != ecintmap_end (modules))
  {
    EcMessageModule* item = ecintmap_data (node);
    
    if (isAssigned (item->fct))
    {
      ret = item->fct (item->ptr, dIn, dOut);
    }
  }
  
  return ret;  
}

//----------------------------------------------------------------------------------------

int ecmessages_send (uint_t module, uint_t method, EcMessageData* dIn, EcMessageData* dOut)
{
  EcMessages self = getGlobalMessages ();

  int ret = ENTC_RESCODE_NOT_FOUND;  
  EcIntMapNode node;

  ecreadwritelock_lockRead (self->mutex);

  node = ecintmap_find (self->functions, method);
  if (node != ecintmap_end (self->functions))
  {
    ret = ecmessages_send_next (ecintmap_data (node), module, dIn, dOut);
  }
  ecreadwritelock_unlockRead (self->mutex);
  
  return ret;  
}

//----------------------------------------------------------------------------------------

void ecmessages_initData (EcMessageData* data, uint_t type, uint_t rev)
{
  data->type = type;
  data->rev = rev;

  data->content = NULL;
  data->ref = 0;
}

//----------------------------------------------------------------------------------------
