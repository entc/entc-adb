#ifndef __ENTC_STC__STREAM__H
#define __ENTC_STC__STREAM__H 1

// cape includes
#include "sys/entc_export.h"
#include "sys/entc_types.h"
#include "stc/entc_str.h"

//=============================================================================

struct EntcStream_s; typedef struct EntcStream_s* EntcStream;

//-----------------------------------------------------------------------------

__ENTC_LIBEX EntcStream      entc_stream_new (void);

__ENTC_LIBEX void            entc_stream_del (EntcStream*);

__ENTC_LIBEX void            entc_stream_clr (EntcStream);

__ENTC_LIBEX const char*     entc_stream_get (EntcStream);

__ENTC_LIBEX number_t        entc_stream_size (EntcStream);

__ENTC_LIBEX const char*     entc_stream_data (EntcStream);

//-----------------------------------------------------------------------------
// convert to other types

__ENTC_LIBEX EntcString      entc_stream_to_str (EntcStream*);

__ENTC_LIBEX number_t        entc_stream_to_n (EntcStream);

__ENTC_LIBEX EntcString      entc_stream_to_s (EntcStream);

//-----------------------------------------------------------------------------
// append functions

__ENTC_LIBEX void            entc_stream_append_str (EntcStream, const char*);

__ENTC_LIBEX void            entc_stream_append_buf (EntcStream, const char*, unsigned long size);

__ENTC_LIBEX void            entc_stream_append_fmt (EntcStream, const char*, ...);

__ENTC_LIBEX void            entc_stream_append_c (EntcStream, char);

__ENTC_LIBEX void            entc_stream_append_n (EntcStream, number_t);

__ENTC_LIBEX void            entc_stream_append_f (EntcStream, double);

__ENTC_LIBEX void            entc_stream_append_stream (EntcStream, EntcStream);

//-----------------------------------------------------------------------------

#endif
