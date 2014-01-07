#include "blank.h"
#include "lauxlib.h"
#include "lua.h"
#include "SDL.h"

/**Private********************************************************************/

/* This function allows Lua to index a userdata like it is an ordinary table
   The userdata's environment table is used as a proxy to store its fields */
static int GetUserdataField(lua_State *L)
{
	/* Check if the key is in the environment */
	lua_getfenv(L, 1);
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);

	/* If the key does not exist key into the userdata's metatable */
	if ( lua_isnil(L, -1) ) {
		lua_pop(L, 2);
		lua_rawget(L, lua_upvalueindex(1));
	}
	return 1;
}

/* This function allows Lua to set a new index onto a userdata like it is an
   ordinary table. New keys are inserted into the userdata's environment */
static int SetUserdataField(lua_State *L)
{
	lua_getfenv(L, 1);
	lua_insert(L, 2);
	lua_rawset(L, -3);
	return 0;
}

/**Public*********************************************************************/

int SDLua_Error(lua_State *L)
{
	lua_pushnil(L);
	lua_pushstring(L, SDL_GetError());
	return 2;
}

int SDLua_Fail(lua_State *L)
{
	lua_pushstring(L, SDL_GetError());
	lua_error(L);
	return 1;
}

int SDLua_Self(lua_State *L)
{
	lua_settop(L, 1);
	return 1;
}

SDLua_Factory SDLua_CreateFactory(lua_State *L, size_t size,
																	const luaL_reg metatable[])
{
	lua_newtable(L);
	lua_pushnumber(L, size);
	lua_setfield(L, -2, "size");
	lua_newtable(L);
	lua_setfield(L, -2, "allocations");
	if ( metatable ) {
		lua_newtable(L);
		lua_pushliteral(L, "__index");
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, GetUserdataField, 1);
		lua_rawset(L, -3);
		lua_pushliteral(L, "__newindex");
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, SetUserdataField, 1);
		lua_rawset(L, -3);
		luaL_register(L, NULL, metatable);
		lua_setfield(L, -2, "metatable");
	}
	return luaL_ref(L, LUA_ENVIRONINDEX);
}

int SDLua_GetResourceMetatable(lua_State *L, SDLua_Factory factory)
{
	lua_rawgeti(L, LUA_ENVIRONINDEX, factory);
	lua_getfield(L, -1, "metatable");
	lua_remove(L, -2);
	if ( lua_istable(L, -1) ) {
		return 1;
	}
	lua_pop(L, 1);
	return 0;
}

void *SDLua_CreateResource(lua_State *L, SDLua_Factory factory)
{
	size_t size = 0;
	void *resourceptr = NULL;

	/* Get the size of the allocation */
	lua_rawgeti(L, LUA_ENVIRONINDEX, factory);
	lua_getfield(L, -1, "size");
	size = (size_t)lua_tonumber(L, -1);
	lua_pop(L, 1);

	/* Create the userdata and attach the metatable */
	resourceptr = lua_newuserdata(L, size);
	lua_getfield(L, -2, "metatable");
	if ( lua_istable(L, -1) ) {
		lua_setmetatable(L, -2);
	} else {
		lua_pop(L, 1);
	}

	/* Unstack the factory */
	lua_remove(L, -2);

	/* Attach a new environment to the userdata */
	lua_newtable(L);
	lua_setfenv(L, -2);

	return resourceptr;
}

void SDLua_CloseFactory(lua_State *L, SDLua_Factory factory)
{
	lua_rawgeti(L, LUA_ENVIRONINDEX, factory);
	lua_getfield(L, -1, "allocations");
	for ( lua_pushnil(L); lua_next(L, -2); lua_pop(L, 0) ) {
		lua_getfield(L, -1, "dispose");
		lua_insert(L, -2);
		lua_call(L, 1, 0);
	}
	lua_pop(L, 2);
}
