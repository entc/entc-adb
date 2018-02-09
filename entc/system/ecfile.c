#include "ecfile.h"

#include "system/macros.h"

#include <string.h>

/*------------------------------------------------------------------------*/

int ecdh_scan (const EcString path, EcList entries, int filetype)
{
  /* variables */
  EcDirHandle dh = 0;
  EcFileInfo info;
  
  dh = ecdh_create (path);
  /* was the open successful */
  
  if(!dh)
  {
    return FALSE;
  }
  
  while (ecdh_next(dh, &info, FALSE))
  {
    if (info->type == ENTC_FILETYPE_ISNONE)
    {
      // scan only again if the type is not clear (on netfs?)
      ecdh_seekType (path, info);
    }
    
    if (info->type == filetype)
    {
      eclist_push_back (entries, ecfs_mergeToPath(path, info->name));
    }
  }

  ecdh_destroy (&dh);
  
  return TRUE;
}

/*------------------------------------------------------------------------*/

EcString ecfs_mergeToPath(const EcString path, const EcString file)
{
  if (*file == ENTC_PATH_SEPARATOR) {    
    return ecstr_cat2(path, file);
  } else {
    return ecstr_catc(path, ENTC_PATH_SEPARATOR, file);
  }
}

/*------------------------------------------------------------------------*/

const EcString ecfs_extractFile(const EcString path)
{
  const EcString pos = strrchr( path, ENTC_PATH_SEPARATOR );
  
  if( pos )
  {
    return pos + 1;
  }
  return path;
}

/*------------------------------------------------------------------------*/

EcString ecfs_getDirectory(const EcString filename)
{
  /* variables */
  const char* pos;

  if( !filename )
  {
    return ecstr_init();
  }
  
  pos = strrchr(filename, ENTC_PATH_SEPARATOR);
  
  if( !pos )
  {
    return ecstr_init();    
  }

  return ecstr_part(filename, pos - filename);
}

//-----------------------------------------------------------------------------------

const EcString ecfs_extractFileExtension(const EcString path)
{
  /* variables */
  const EcString pos;
  const EcString filename;
  
  filename = ecfs_extractFile(path);
  
  if( !ecstr_valid(filename) )
  {
    return 0;  
  }
  
  pos = strrchr( filename, '.' );
  
  if( pos )
  {
    return pos + 1;
  }
  
  return 0;
}

//-----------------------------------------------------------------------------------

EcString ecfs_extractFileName(const EcString path)
{
  /* variables */
  const EcString pos;
  const EcString filename;
  
  filename = ecfs_extractFile(path);
  
  if( !ecstr_valid(filename) )
  {
    return 0;  
  }
  
  pos = strrchr( filename, '.' );
  
  if( pos )
  {
    return ecstr_part (filename, (pos - filename));
  }
  else
  {
    return ecstr_copy (filename);
  }
}

//-----------------------------------------------------------------------------------

void ecfi_clone (const EcFileInfo source, EcFileInfo dest)
{
  memcpy (dest, source, sizeof(EcFileInfo_s));  
  dest->name = ecstr_copy (source->name);  
}

//-----------------------------------------------------------------------------------
