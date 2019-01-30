#ifndef __ENTC_FMT__TOKENIZER__H
#define __ENTC_FMT__TOKENIZER__H 1

#include "sys/entc_export.h"
#include "sys/entc_types.h"
#include "stc/entc_list.h"
#include "stc/entc_str.h"

//-----------------------------------------------------------------------------

                                  /* returns a list of EntcString's */
__ENTC_LIBEX   EntcList           entc_tokenizer_buf           (const char* buffer, number_t len, char token);

                                  /* returns TRUE / FALSE if successfull */
__ENTC_LIBEX   int                entc_tokenizer_split         (const EntcString source, char token, EntcString* p_left, EntcString* p_right);

//-----------------------------------------------------------------------------

#endif


