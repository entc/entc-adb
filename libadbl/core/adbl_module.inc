#include <system/macros.h>
#include "adbl_structs.h"
#include "adbl.h"

void*        adblmodule_dbconnect (AdblConnectionProperties* properties, EcLogger logger);
void         adblmodule_dbdisconnect (void* ptr, EcLogger logger);
void*        adblmodule_dbquery (void* ptr, AdblQuery* query, EcLogger logger);
uint_t       adblmodule_dbtable_size (void* ptr, const EcString name, EcLogger logger);
int          adblmodule_dbupdate (void* ptr, AdblUpdate* update, EcLogger logger);
int          adblmodule_dbinsert (void* ptr, AdblInsert* insert, EcLogger logger);
int          adblmodule_dbdelete (void* ptr, AdblDelete* delete, EcLogger logger);
void         adblmodule_dbbegin (void* ptr, EcLogger logger);
void         adblmodule_dbcommit (void* ptr, EcLogger logger);
void         adblmodule_dbrollback (void* ptr, EcLogger logger);
int          adblmodule_dbcursor_next (void* ptr);
const char*  adblmodule_dbcursor_data (void* ptr, uint_t column);
const char*  adblmodule_dbcursor_nextdata (void* ptr);
void         adblmodule_dbcursor_release (void* ptr);
void*        adblmodule_dbsequence_get (void* ptr, const EcString name, EcLogger logger);
void         adblmodule_dbsequence_release (void* ptr, EcLogger logger);
uint_t       adblmodule_dbsequence_next (void* ptr, EcLogger logger);
EcList       adblmodule_dbschema ( void* ptr, EcLogger logger ); 
AdblTable*   adblmodule_dbtable ( void* ptr, const EcString, EcLogger logger );


__CPP_EXTERN______________________________________________________________________________START

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbinfo=_dbinfo@0")
#endif
const AdblModuleInfo* _STDCALL dbinfo (void)
{
  return &ModuleInfo;
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbconnect=_dbconnect@8")
#endif
void* _STDCALL dbconnect (AdblConnectionProperties* properties, EcLogger logger)
{
  return adblmodule_dbconnect (properties, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbdisconnect=_dbdisconnect@8")
#endif
void _STDCALL dbdisconnect (void* ptr, EcLogger logger)
{
  adblmodule_dbdisconnect (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbquery=_dbquery@12")
#endif
void* _STDCALL dbquery (void* ptr, AdblQuery* query, EcLogger logger)
{
  return adblmodule_dbquery (ptr, query, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbtable_size=_dbtable_size@12")
#endif
uint_t _STDCALL dbtable_size (void* ptr, const EcString name, EcLogger logger)
{
  return adblmodule_dbtable_size (ptr, name, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbupdate=_dbupdate@12")
#endif
int _STDCALL dbupdate (void* ptr, AdblUpdate* update, EcLogger logger)
{
  return adblmodule_dbupdate (ptr, update, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbinsert=_dbinsert@12")
#endif
int _STDCALL dbinsert (void* ptr, AdblInsert* insert, EcLogger logger)
{
  return adblmodule_dbinsert (ptr, insert, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbdelete=_dbdelete@12")
#endif
int _STDCALL dbdelete (void* ptr, AdblDelete* delete, EcLogger logger)
{
  return adblmodule_dbdelete (ptr, delete, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbbegin=_dbbegin@8")
#endif
void _STDCALL dbbegin (void* ptr, EcLogger logger)
{
  adblmodule_dbbegin (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcommit=_dbcommit@8")
#endif
void _STDCALL dbcommit (void* ptr, EcLogger logger)
{
  adblmodule_dbcommit (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbrollback=_dbrollback@8")
#endif
void _STDCALL dbrollback (void* ptr, EcLogger logger)
{
  adblmodule_dbrollback (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_next=_dbcursor_next@4")
#endif
int _STDCALL dbcursor_next (void* ptr)
{
  return adblmodule_dbcursor_next (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_data=_dbcursor_data@8")
#endif
const char* _STDCALL dbcursor_data (void* ptr, uint_t column)
{
  return adblmodule_dbcursor_data (ptr, column);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_nextdata=_dbcursor_nextdata@4")
#endif
const char* _STDCALL dbcursor_nextdata (void* ptr)
{
  return adblmodule_dbcursor_nextdata (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_release=_dbcursor_release@4")
#endif
void _STDCALL dbcursor_release (void* ptr)
{
  adblmodule_dbcursor_release (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbsequence_get=_dbsequence_get@12")
#endif
void* _STDCALL dbsequence_get (void* ptr, const EcString name, EcLogger logger)
{
  return adblmodule_dbsequence_get (ptr, name, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbsequence_release=_dbsequence_release@8")
#endif
void _STDCALL dbsequence_release (void* ptr, EcLogger logger)
{
  adblmodule_dbsequence_release (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbsequence_next=_dbsequence_next@8")
#endif
uint_t _STDCALL dbsequence_next (void* ptr, EcLogger logger)
{
  return adblmodule_dbsequence_next (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbdbschema=_dbschema@8")
#endif
EcList _STDCALL dbschema ( void* ptr, EcLogger logger )
{
  return adblmodule_dbschema (ptr, logger);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbtable=_dbtable@12")
#endif
AdblTable* _STDCALL dbtable ( void* ptr, const EcString tablename, EcLogger logger )
{
  return adblmodule_dbtable (ptr, tablename, logger);
}

__CPP_EXTERN______________________________________________________________________________END
