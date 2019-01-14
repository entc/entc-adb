#include "ecfile.h"

#include "system/macros.h"

#include <string.h>

/*------------------------------------------------------------------------*/

int ecdh_scan (const EcString path, EntcList entries, int filetype)
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
      entc_list_push_back (entries, ecfs_mergeToPath(path, info->name));
    }
  }

  ecdh_destroy (&dh);
  
  return TRUE;
}

//--------------------------------------------------------------------------------

uint64_t ecdh_size (EcDirHandle self, int recursive)
{
  EcFileInfo info;
  uint64_t size = 0;
  
  while (ecdh_next (self, &info, TRUE))
  {
    if (ecstr_equal(info->name, ".") || ecstr_equal(info->name, ".."))
    {
      continue;
    }
    
    switch (info->type)
    {
      case ENTC_FILETYPE_ISDIR:
      {
        if (recursive)
        {
          EcString subdir = ecfs_mergeToPath (ecdh_path (self), info->name);
          
          EcDirHandle dh = ecdh_create (subdir);
          if (dh)
          {
            size += ecdh_size (dh, recursive);
            
            ecdh_destroy (&dh);
          }
          
          ecstr_delete (&subdir);
        }
        
        break;
      }
      case ENTC_FILETYPE_ISFILE:
      {
        size += info->size;
        
        break;
      }
    }
  }
  
  return size;
}

//--------------------------------------------------------------------------------
// as long there is no OS specific way, the generic implementation follows

// forward declaration
int ecfs_copy_file_directory (EcFileInfo finfo, const EcString source, const EcString dest, EcErr err);

//--------------------------------------------------------------------------------

int ecfs_copy_directory (EcDirHandle dh, const EcString source, const EcString dest, EcErr err)
{
  EcFileInfo finfo;
  
  // iterate through all files
  while (ecdh_next (dh, &finfo, TRUE))
  {
    int res;
    
    // ignore these 
    if (ecstr_equal (finfo->name, ".") || ecstr_equal (finfo->name, ".."))
    {
      continue;
    }

    {
      EcString found_source = ecfs_mergeToPath (source, finfo->name);
      EcString found_dest = ecfs_mergeToPath (dest, finfo->name);
      
      res = ecfs_copy_file_directory (finfo, found_source, found_dest, err);
      
      ecstr_delete (&found_source);
      ecstr_delete (&found_dest);
    }
    
    if (res)
    {
      return res;
    }
  }
 
  return ENTC_ERR_NONE;
}

//--------------------------------------------------------------------------------

int ecfs_copy_file_directory (EcFileInfo finfo, const EcString source, const EcString dest, EcErr err)
{
  switch (finfo->type)
  {
    case ENTC_FILETYPE_ISDIR:
    {
      EcDirHandle dh;
        
      // create directory
      if (!ecfs_createDirIfNotExists (dest))
      {
        return ecerr_lastErrorOS(err, ENTC_LVL_ERROR);
      }      
            
      dh = ecdh_create (source);
      if (dh)
      {
        int res = ecfs_copy_directory (dh, source, dest, err);
        
        // clean up
        ecdh_destroy (&dh);
        
        return res;
      }
      else
      {
        return ecerr_lastErrorOS(err, ENTC_LVL_ERROR);
      }
    }
    case ENTC_FILETYPE_ISFILE:
    {
      return ecfs_cpfile (source, dest, err);
    }
  }  
  
  return ENTC_ERR_NONE;
}

//--------------------------------------------------------------------------------

int ecfs_cpdir (const EcString source, const EcString dest, EcErr err)
{
  EcFileInfo_s sinfo;
  
  // get the fileinfo of the source
  if (!ecfs_fileInfo (&sinfo, source))
  {
    return ecerr_lastErrorOS(err, ENTC_LVL_ERROR);
  }
  
  return ecfs_copy_file_directory (&sinfo, source, dest, err);
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
