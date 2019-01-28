#include <sys/entc_export.h>
#include "adbl_structs.h"
#include "adbl.h"

void*        adblmodule_dbconnect (AdblConnectionProperties* properties);
void         adblmodule_dbdisconnect (void* ptr);
void*        adblmodule_dbquery (void* ptr, AdblQuery* query);
int          adblmodule_dbprocedure (void* ptr, AdblProcedure* proc);
uint_t       adblmodule_dbtable_size (void* ptr, const EcString name);
int          adblmodule_dbupdate (void* ptr, AdblUpdate* update, int insert);
int          adblmodule_dbinsert (void* ptr, AdblInsert* insert);
int          adblmodule_dbdelete (void* ptr, AdblDelete* delete);
void         adblmodule_dbbegin (void* ptr);
void         adblmodule_dbcommit (void* ptr);
void         adblmodule_dbrollback (void* ptr);
int          adblmodule_dbcursor_next (void* ptr);
const char*  adblmodule_dbcursor_data (void* ptr, uint_t column);
const char*  adblmodule_dbcursor_nextdata (void* ptr);
void         adblmodule_dbcursor_release (void* ptr);
void*        adblmodule_dbsequence_get (void* ptr, const EcString name);
void         adblmodule_dbsequence_release (void* ptr);
uint_t       adblmodule_dbsequence_next (void* ptr);
EcList       adblmodule_dbschema ( void* ptr ); 
AdblTable*   adblmodule_dbtable ( void* ptr, const EcString );


#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbinfo=_dbinfo@0")
#endif
const AdblModuleInfo* __STDCALL dbinfo (void)
{
  return &ModuleInfo;
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbconnect=_dbconnect@4")
#endif
void* __STDCALL dbconnect (AdblConnectionProperties* properties)
{
  return adblmodule_dbconnect (properties);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbdisconnect=_dbdisconnect@4")
#endif
void __STDCALL dbdisconnect (void* ptr)
{
  adblmodule_dbdisconnect (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbquery=_dbquery@8")
#endif
void* __STDCALL dbquery (void* ptr, AdblQuery* query)
{
  return adblmodule_dbquery (ptr, query);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbprocedure=_dbprocedure@8")
#endif
int __STDCALL dbprocedure (void* ptr, AdblProcedure* procedure)
{
  return adblmodule_dbprocedure (ptr, procedure);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbtable_size=_dbtable_size@8")
#endif
uint_t __STDCALL dbtable_size (void* ptr, const EcString name)
{
  return adblmodule_dbtable_size (ptr, name);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbupdate=_dbupdate@12")
#endif
int __STDCALL dbupdate (void* ptr, AdblUpdate* update, int insert)
{
  return adblmodule_dbupdate (ptr, update, insert);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbinsert=_dbinsert@8")
#endif
int __STDCALL dbinsert (void* ptr, AdblInsert* insert)
{
  return adblmodule_dbinsert (ptr, insert);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbdelete=_dbdelete@8")
#endif
int __STDCALL dbdelete (void* ptr, AdblDelete* delete)
{
  return adblmodule_dbdelete (ptr, delete);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbbegin=_dbbegin@4")
#endif
void __STDCALL dbbegin (void* ptr)
{
  adblmodule_dbbegin (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcommit=_dbcommit@4")
#endif
void __STDCALL dbcommit (void* ptr)
{
  adblmodule_dbcommit (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbrollback=_dbrollback@4")
#endif
void __STDCALL dbrollback (void* ptr)
{
  adblmodule_dbrollback (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_next=_dbcursor_next@4")
#endif
int __STDCALL dbcursor_next (void* ptr)
{
  return adblmodule_dbcursor_next (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_data=_dbcursor_data@8")
#endif
const char* __STDCALL dbcursor_data (void* ptr, uint_t column)
{
  return adblmodule_dbcursor_data (ptr, column);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_nextdata=_dbcursor_nextdata@4")
#endif
const char* __STDCALL dbcursor_nextdata (void* ptr)
{
  return adblmodule_dbcursor_nextdata (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbcursor_release=_dbcursor_release@4")
#endif
void __STDCALL dbcursor_release (void* ptr)
{
  adblmodule_dbcursor_release (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbsequence_get=_dbsequence_get@8")
#endif
void* __STDCALL dbsequence_get (void* ptr, const EcString name)
{
  return adblmodule_dbsequence_get (ptr, name);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbsequence_release=_dbsequence_release@4")
#endif
void __STDCALL dbsequence_release (void* ptr)
{
  adblmodule_dbsequence_release (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbsequence_next=_dbsequence_next@4")
#endif
uint_t __STDCALL dbsequence_next (void* ptr)
{
  return adblmodule_dbsequence_next (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbdbschema=_dbschema@4")
#endif
EcList __STDCALL dbschema ( void* ptr )
{
  return adblmodule_dbschema (ptr);
}

//________________________________________________________________________________________

#if defined _WIN64 || defined _WIN32
#pragma comment(linker, "/EXPORT:dbtable=_dbtable@8")
#endif
AdblTable* __STDCALL dbtable ( void* ptr, const EcString tablename )
{
  return adblmodule_dbtable (ptr, tablename);
}
