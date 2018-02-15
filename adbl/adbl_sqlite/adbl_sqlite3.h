#ifndef QUOMADBL_SQLITE3_H
#define QUOMADBL_SQLITE3_H 1

#include <system/macros.h>

#include <types/ecstring.h>

#include "adbl.h"

#ifdef __cplusplus
extern "C" {
#endif
  
__LIB_EXPORT const AdblModuleInfo* adblPlugin(void);
  
__LIB_EXPORT struct QDBLExeMatrix* getQDBLExeMatrix(void);
  
#ifdef __cplusplus
}
#endif

#endif
