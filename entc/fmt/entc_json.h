#ifndef __ENTC_FMT__JSON__H
#define __ENTC_FMT__JSON__H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"
#include "stc/entc_udc.h"

//-----------------------------------------------------------------------------

__ENTC_LIBEX   EntcUdc           entc_json_from_s           (const EntcString, const EntcString name);

__ENTC_LIBEX   EntcUdc           entc_json_from_buf         (const char* buffer, number_t size, const EntcString name);

__ENTC_LIBEX   EntcString        entc_json_to_s             (const EntcUdc source);

//-----------------------------------------------------------------------------

#endif

