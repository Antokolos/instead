#ifndef _LUAPASSING_H
#define _LUAPASSING_H
#include <lua.hpp>

#if LUA_VERSION_NUM >= 502  
#  define new_lib(L, l) (lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1), luaL_setfuncs(L,l,0))
#else
#  define new_lib(L, l) (luaL_register(L, "statsAPI", l))
#endif

#endif