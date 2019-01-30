#ifndef __ENTC_FMT__PARSER_JSON__H
#define __ENTC_FMT__PARSER_JSON__H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"
#include "sys/entc_err.h"

//=============================================================================

#define ENTC_JPARSER_UNDEFINED       0
#define ENTC_JPARSER_OBJECT_NODE     1
#define ENTC_JPARSER_OBJECT_LIST     2
#define ENTC_JPARSER_OBJECT_TEXT     3
#define ENTC_JPARSER_OBJECT_NUMBER   4
#define ENTC_JPARSER_OBJECT_FLOAT    5
#define ENTC_JPARSER_OBJECT_BOLEAN   6
#define ENTC_JPARSER_OBJECT_NULL     7

//-----------------------------------------------------------------------------

struct EntcParserJson_s; typedef struct EntcParserJson_s* EntcParserJson;

// callbacks
typedef void   (__STDCALL *fct_parser_json_onItem)          (void* ptr, void* obj, int type, void* val, const char* key, int index);
typedef void*  (__STDCALL *fct_parser_json_onObjNew)        (void* ptr, int type);
typedef void   (__STDCALL *fct_parser_json_onObjDel)        (void* ptr, void* obj);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcParserJson    entc_parser_json_new       (void* ptr, fct_parser_json_onItem, fct_parser_json_onObjNew, fct_parser_json_onObjDel);

__ENTC_LIBEX   void              entc_parser_json_del       (EntcParserJson*);

//-----------------------------------------------------------------------------

__ENTC_LIBEX   int               entc_parser_json_process   (EntcParserJson, const char* buffer, number_t size, EntcErr err);

__ENTC_LIBEX   void*             entc_parser_json_object    (EntcParserJson);

//-----------------------------------------------------------------------------

#endif
