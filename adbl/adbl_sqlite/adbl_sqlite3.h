#ifndef QUOMADBL_SQLITE3_H
#define QUOMADBL_SQLITE3_H 1

#include <sys/entc_export.h>
#include <types/ecstring.h>

#include "adbl.h"

__ENTC_LIBEX const AdblModuleInfo* adblPlugin(void);
  
__ENTC_LIBEX struct QDBLExeMatrix* getQDBLExeMatrix(void);
  
#endif
