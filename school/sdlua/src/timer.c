#include "timer.h"
#include "lauxlib.h"
#include "lua.h"
#include "SDL.h"
#include "SDLua.h"

/**Public*********************************************************************/

static int timer_delay(lua_State *L)
{
	SDL_Delay((Uint32)(1000 * lua_tonumber(L, 1)));
	return 0;
}

static int timer_getticks(lua_State *L)
{
	lua_pushnumber(L, SDL_GetTicks());
	return 1;
}

/**Initialization*************************************************************/

void SDLua_OpenTimerModule(lua_State *L)
{
	const luaL_reg kInterface[] = {
		{"delay", timer_delay},
		{"getticks", timer_getticks},
		{0}
	};

	SDLua_PushPackage(L);
	luaL_register(L, NULL, kInterface);
	lua_pop(L, 1);
}
