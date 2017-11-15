#include "eclua.h"

// lua includes
#include "lua.h"
#include "lauxlib.h"

// entc includes
#include "tools/ecjparser.h"
#include "system/ecdlib.h"

//---------------------------------------------------------------------------

void eclua_push_fromJson_push (lua_State *L, const char* key, int index)
{
  if (key)
  {
    lua_setfield (L, -2, key);
  }
  else
  {
    lua_pushinteger (L, index + 1);
    lua_rotate (L, -2, 1);
    
    lua_settable (L, -3);
  }
}

//---------------------------------------------------------------------------

static void __STDCALL eclua_push_fromJson_onItem (void* ptr, void* obj, int type, void* val, const char* key, int index)
{
  lua_State *L = ptr;
  
  switch (type)
  {
    case ENTC_JPARSER_OBJECT_NODE:
    case ENTC_JPARSER_OBJECT_LIST:
    {
      // table is already pushed on top of the stack
      eclua_push_fromJson_push (L, key, index);
      break;
    }
    case ENTC_JPARSER_OBJECT_TEXT:
    {
      lua_pushstring(L, val);
      eclua_push_fromJson_push (L, key, index);
      break;
    }
    case ENTC_JPARSER_OBJECT_NUMBER:
    {
      
      break;
    }
    case ENTC_JPARSER_OBJECT_FLOAT:
    {
      
      break;
    }
    case ENTC_JPARSER_OBJECT_BOLEAN:
    {
      
      break;
    }
    case ENTC_JPARSER_OBJECT_NULL:
    {
      
      break;
    }
  }
}

//---------------------------------------------------------------------------

static void* __STDCALL eclua_push_fromJson_onCreate (void* ptr, int type)
{
  lua_State *L = ptr;
  
  // creates a new table and pushes it onto the stack
  lua_createtable(L, 0, 0);
  
  return NULL;
}

//---------------------------------------------------------------------------

static void __STDCALL eclua_push_fromJson_onDestroy (void* ptr, void* obj)
{
  //lua_State *L = ptr;
}

//---------------------------------------------------------------------------

int eclua_fct_push_fromJson (lua_State *L, const char* buffer, uint64_t len, EcErr err)
{
  int res;
  EcJsonParser jp = ecjsonparser_create (eclua_push_fromJson_onItem, eclua_push_fromJson_onCreate, eclua_push_fromJson_onDestroy, L);
  
  // parse
  res = ecjsonparser_parse (jp, buffer, len, err);
  
  ecjsonparser_destroy (&jp);
  
  return res;
}

//---------------------------------------------------------------------------

void eclua_fct_toJson (lua_State *L, int idx, EcStream stream)
{
  int type = lua_type (L, idx);
  switch (type)
  {
    case LUA_TNIL:
    {
      ecstream_append_str (stream, "NULL");
      break;
    }
    case LUA_TBOOLEAN:
    {
      ecstream_append_str (stream, lua_toboolean (L, idx) ? "true" : "false");
      break;
    }
    case LUA_TNUMBER:
    {
      if (lua_isinteger (L, idx))
      {
        int val = lua_tointegerx (L, idx, NULL);
        ecstream_append_i (stream, val);
      }
      else if (lua_isnumber (L, idx))
      {
        int val = lua_tonumberx (L, idx, NULL);
        ecstream_append_i (stream, val);
      }
      
      break;
    }
    case LUA_TSTRING:
    {
      const char* content;
      
      content = lua_tolstring (L, idx, NULL);
      
      ecstream_append_c (stream, '"');
      ecstream_append_str (stream, content);
      ecstream_append_c (stream, '"');
      
      break;
    }
    case LUA_TTABLE:
    {
      int i;
      
      lua_pushvalue (L, idx);
      lua_pushnil (L);
      
      ecstream_append_c (stream, '{');
      
      // convert from lua to json
      for (i = 0; lua_next(L, -2); i++)
      {
        const char* key;
        
        if (i > 0)
        {
          ecstream_append_c (stream, ',');
        }
        
        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue (L, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
        
        key = lua_tolstring (L, -1, NULL);
        
        ecstream_append_c (stream, '"');
        ecstream_append_str (stream, key);
        ecstream_append_c (stream, '"');
        ecstream_append_c (stream, ':');
        
        eclua_fct_toJson (L, -2, stream);
        
        // pop value + copy of key, leaving original key
        lua_pop(L, 2);
        // stack now contains: -1 => key; -2 => table
      }
      
      ecstream_append_c (stream, '}');
      
      lua_pop(L, 1);
      
      break;
    }
    default:
    {
      
    }
  }
}

//---------------------------------------------------------------------------

typedef void*  __STDCALL (*fct_tmodule_init)       (void);
typedef void   __STDCALL (*fct_tmodule_done)       (void*);
typedef int    __STDCALL (*fct_tmodule_method)     (void*, const char* method, const char* paramsInJson, char** resBuffer);
typedef void   __STDCALL (*fct_tmodule_freeBuffer) (char** resBuffer);

//---------------------------------------------------------------------------

typedef struct {
  
  fct_tmodule_init init;
  fct_tmodule_done done;
  fct_tmodule_method method;
  fct_tmodule_freeBuffer freeBuffer;
  
  EcDl tmodule;
  void* ptr;
  
} LueTModule;

//---------------------------------------------------------------------------

#define LUA_TMODULE "TModule"

//---------------------------------------------------------------------------

static int eclua_sleep (lua_State *L)
{
  int m = luaL_checknumber(L,1);
  
#ifdef _WIN32
  
  Sleep(m);
  
#else
  
  wait (m);
  
#endif
  
  return 0;
}

//---------------------------------------------------------------------------

static int eclua_tmodule_create (lua_State *L)
{
  const char* moduleName = lua_tolstring(L, 1, NULL);
  
  LueTModule* lm = lua_newuserdata(L, sizeof(LueTModule));
  luaL_setmetatable(L, LUA_TMODULE);
  
  // try to load the module
  {
    int res;
    EcErr err = ecerr_create ();
    lm->tmodule = ecdl_create (moduleName, 0);
    
    res = ecdl_load (lm->tmodule, err);
    if (res)
    {
      printf ("ERROR: %s\n", err->text);
      return 0;
    }
    
    res = ecdl_assign (lm->tmodule, err, lm, 4, "tmodule_init", "tmodule_done", "tmodule_method", "tmodule_freeBuffer");
    if (res)
    {
      printf ("ERROR: %s\n", err->text);
      return 0;
    }
    
    lm->ptr = lm->init();
    
    ecerr_destroy (&err);
  }
  
  return 1;
}

//---------------------------------------------------------------------------

static int eclua_tmodule_destroy (lua_State *L)
{
  LueTModule* lm = lua_touserdata(L, 1);
  
  lm->done (lm->ptr);
  
  ecdl_destroy (&(lm->tmodule));
  
  return 0;
}

//---------------------------------------------------------------------------

static int eclua_tmodule_method (lua_State *L)
{
  LueTModule* lm = lua_touserdata(L, 1);
  
  const char* method = lua_tolstring(L, 2, NULL);
  char* result = NULL;
  
  int type = lua_type(L, 3);
  switch (type)
  {
    case LUA_TTABLE:
    {
      EcStream stream = ecstream_create ();
      
      // convert from lua to json
      eclua_fct_toJson (L, 3, stream);
      
      lm->method (lm->ptr, method, ecstream_get(stream), &result);
      
      //printf("%s", stdstream_get(stream));
      
      ecstream_destroy (&stream);
      break;
    }
    case LUA_TSTRING:
    {
      const char* params = lua_tolstring(L, 3, NULL);
      
      if (params)
      {
        //    lm->method (lm->ptr, method, params, &result);
      }
      break;
    }
    default:
    {
      
    }
  }
  
  if (result)
  {
    EcErr err = ecerr_create();
    
    // convert from json text to lua tables
    eclua_fct_push_fromJson (L, result, ecstr_len(result), err);
    
    lm->freeBuffer (&result);
    
    ecerr_destroy(&err);
    
    return 1;
  }
  else
  {
    return 0;
  }
}

//---------------------------------------------------------------------------

static void eclua_add_tmodule (lua_State* L)
{
  // register constructor
  lua_pushcfunction (L, eclua_tmodule_create);
  lua_setglobal(L, LUA_TMODULE);
  
  // register destructor
  luaL_newmetatable(L, LUA_TMODULE);
  lua_pushcfunction(L, eclua_tmodule_destroy); lua_setfield(L, -2, "__gc");
  
  lua_pushvalue(L, -1); lua_setfield(L, -2, "__index");
  
  // register method
  lua_pushcfunction(L, eclua_tmodule_method); lua_setfield(L, -2, "method");
  
  lua_pop(L, 1);
}

//---------------------------------------------------------------------------

struct EcLua_s
{
  
  lua_State *L;
  
  //EcLuaFunctions funct;
  
  //EcDl lualib;
  
};

//---------------------------------------------------------------------------

EcLua eclua_create ()
{
  EcLua self = ENTC_NEW(struct EcLua_s);
  
  return self;
}

//---------------------------------------------------------------------------

void eclua_destroy (EcLua* pself)
{
  
}

//---------------------------------------------------------------------------

int eclua_init (EcLua self, EcErr err)
{
  // all lua contextes are held in this structure
  self->L = luaL_newstate ();
  
  // load lua libraries
  luaL_openlibs(self->L);
  
  // register tmodule
  eclua_add_tmodule (self->L);
  
  // register methods
  lua_pushcfunction (self->L, eclua_sleep);
  lua_setglobal(self->L, "sleep");

  return ENTC_ERR_NONE;
}

//---------------------------------------------------------------------------
