#ifndef SDLua_h
#define SDLua_h

#include "lua.h"

/* Calls a function when the package closes (is gc'ed). The parameter function
   is guaranteed SDL has not terminated before it is called. The function will
	 be pushed as a closure if parameter upvalues is positive using the current
	 stack contents. The parameter function must not call SDL_Quit */
extern void SDLua_OnExit(lua_State *L, lua_CFunction function, int upvalues);

/* Pushes the package table onto the Lua stack */
extern void SDLua_PushPackage(lua_State *L);

#endif
