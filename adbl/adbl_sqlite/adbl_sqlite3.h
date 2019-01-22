#ifndef QUOMADBL_SQLITE3_H
#define QUOMADBL_SQLITE3_H 1



#include <types/ecstring.h>

#include "adbl.h"

#ifdef __cplusplus
extern "C" {
#endif
  
__ENTC_LIBEX const AdblModuleInfo* adblPlugin(void);
  
__ENTC_LIBEX struct QDBLExeMatrix* getQDBLExeMatrix(void);
  
#ifdef __cplusplus
}
#endif

#endif
