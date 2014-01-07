/* This module defines the package's initialization and finalization
   It also provides functions for hooking the package finalization phase */

#include "SDLua.h"
#include "lauxlib.h"
#include "lua.h"
#include "SDL.h"
#include "audio.h"
#include "blank.h"
#include "input.h"
#include "timer.h"
#include "video.h"

static int package = LUA_REFNIL;
static int finalizers = LUA_REFNIL;

/**Private********************************************************************/

static int ClosePackage(lua_State *L)
{
	for ( lua_pushnil(L); lua_next(L, lua_upvalueindex(1)); lua_pop(L, 0) ) {
		lua_call(L, 0, 0);
	}
	SDL_Quit();
	return 0;
}

/**Public*********************************************************************/

void SDLua_OnExit(lua_State *L, lua_CFunction function, int upvalues)
{
	if ( upvalues ) {
		lua_pushcclosure(L, function, upvalues);
	} else {
		lua_pushcfunction(L, function);
	}

	lua_rawgeti(L, LUA_ENVIRONINDEX, finalizers);
	lua_insert(L, -2);
	lua_rawseti(L, -2, (int)lua_objlen(L, -2) + 1);
	lua_pop(L, 1);
}

void SDLua_PushPackage(lua_State *L)
{
	lua_rawgeti(L, LUA_ENVIRONINDEX, package);
}

/**Initialization*************************************************************/

int luaopen_SDLua(lua_State *L)
{
	if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
		return SDLua_Fail(L);
	}

	/* Create a new environment */
	lua_newtable(L);
	lua_replace(L, LUA_ENVIRONINDEX);

	/* Create a finalizer to close the package by using a userdata stored in the
	   environment. Create a function table used during that finalizer and store
		 it in the environment and closured onto the finalizer function itself */
	lua_newtable(L);
	finalizers = luaL_ref(L, LUA_ENVIRONINDEX);
	lua_newuserdata(L, sizeof(lua_Number));
	lua_createtable(L, 1, 0);
	lua_rawgeti(L, LUA_ENVIRONINDEX, finalizers);
	lua_pushcclosure(L, ClosePackage, 1);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
	luaL_ref(L, LUA_ENVIRONINDEX);

	/* Create the package table and store it in the environment */
	lua_newtable(L);
	package = luaL_ref(L, LUA_ENVIRONINDEX);
	SDLua_OpenInputModule(L);
	SDLua_OpenTimerModule(L);
	SDLua_OpenAudioModule(L);
	SDLua_OpenVideoModule(L);

	SDLua_PushPackage(L);
	return 1;
}
