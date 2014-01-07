#include "input.h"
#include "lauxlib.h"
#include "lua.h"
#include "SDL.h"
#include "blank.h"
#include "SDLua.h"

static int hooks = LUA_REFNIL;
static int keys = LUA_REFNIL;

/**Private********************************************************************/

static SDLKey KeyNameToSymbol(lua_State *L)
{
	SDLKey key;
	lua_rawgeti(L, LUA_ENVIRONINDEX, keys);
	lua_insert(L, -2);
	lua_rawget(L, -2);
	key = (SDLKey)lua_tonumber(L, -1);
	lua_pop(L, 2);
	return key;
}

static void ConvertTableIntoEvent(lua_State *L, SDL_Event *event)
{
	/* Empty the table */
	for ( lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1) ) {
		lua_pushvalue(L, -2);
		lua_pushnil(L);
		lua_rawset(L, -5);
	}

	switch ( event->type ) {
		/* A quit event is just quit set to true */
		case SDL_QUIT:
			lua_pushliteral(L, "quit");
			lua_pushboolean(L, 1);
			lua_rawset(L, -3);
			break;

		case SDL_KEYDOWN:
			lua_pushliteral(L, "pressed");
			lua_pushstring(L, SDL_GetKeyName(event->key.keysym.sym));
			lua_rawset(L, -3);
			break;

		case SDL_MOUSEMOTION:
			lua_pushliteral(L, "moved");
			lua_pushliteral(L, "mouse");
			lua_rawset(L, -3);
			lua_pushliteral(L, "x");
			lua_pushnumber(L, event->motion.xrel);
			lua_rawset(L, -3);
			lua_pushliteral(L, "y");
			lua_pushnumber(L, event->motion.yrel);
			lua_rawset(L, -3);
			break;

		case SDL_MOUSEBUTTONDOWN:
			lua_pushliteral(L, "pressed");
			lua_pushliteral(L, "mouse");
			lua_rawset(L, -3);
			lua_pushliteral(L, "x");
			lua_pushnumber(L, event->button.x + 1);
			lua_rawset(L, -3);
			lua_pushliteral(L, "y");
			lua_pushnumber(L, event->button.y + 1);
			lua_rawset(L, -3);
			break;

		/* Non-system events are handled here. We lookup the code in the hook 
		   table and if there is a handler we call it with the event table as
			 its only argument */
		default:
			lua_rawgeti(L, LUA_ENVIRONINDEX, hooks);
			lua_rawgeti(L, -1, event->user.code);
			if ( lua_isfunction(L, -1) ) {
				lua_pushlightuserdata(L, event);
				lua_pushvalue(L, -5);
				lua_call(L, 2, 0);
			} else {
				lua_pop(L, 1);
			}
			lua_pop(L, 2);
			break;
	}
}

/* The opposite of Covert; the function converts the Lua table on top of the
   stack into its SDL_Event struct equivalent. */
static SDL_Event RevertTableIntoEvent(lua_State *L)
{
	SDL_Event event = {0};
	int top = lua_gettop(L);

	/* SDL_MOUSEMOTION */
	lua_pushliteral(L, "moved");
	lua_rawget(L, top);
	lua_pushliteral(L, "mouse");
	if ( lua_equal(L, -1, -2) ) {
		lua_pushliteral(L, "x");
		lua_rawget(L, top);
		lua_pushliteral(L, "y");
		lua_rawget(L, top);

		event.type = SDL_MOUSEMOTION;
		event.motion.xrel = (Sint16)lua_tonumber(L, -2);
		event.motion.yrel = (Sint16)lua_tonumber(L, -1);
		goto END;
	} else {
		lua_pop(L, 2);
	}

	/* SDL_KEYDOWN */
	lua_pushliteral(L, "pressed");
	lua_rawget(L, top);
	if ( lua_isnil(L, -1) ) {
		lua_pop(L, 1);
	} else {
		event.type = SDL_KEYDOWN;
		event.key.keysym.sym = KeyNameToSymbol(L);
	}

	/* SDL_QUIT */
	lua_getfield(L, top, "quit");
	if ( lua_isboolean(L, -1) && lua_toboolean(L, -1) ) {
		event.type = SDL_QUIT;
	}

END:
	lua_settop(L, top);
	return event;
}

/**Public*********************************************************************/

int SDLua_SetCustomEventHook(lua_State *L, lua_CFunction function)
{
	int code = 0;
	lua_rawgeti(L, LUA_ENVIRONINDEX, hooks);
	lua_pushcfunction(L, function);
	code = luaL_ref(L, -2);
	lua_pop(L, 1);
	return code;
}

static int input_queue(lua_State *L)
{
	SDL_Event event;
	if ( SDL_PollEvent(&event) ) {
		lua_pushvalue(L, lua_upvalueindex(1));
		ConvertTableIntoEvent(L, &event);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int input_push(lua_State *L)
{
	SDL_Event push;

	luaL_checktype(L, 1, LUA_TTABLE);
	push = RevertTableIntoEvent(L);
	if ( SDL_PushEvent(&push) < 0 ) {
		return SDLua_Error(L);
	}
	lua_pushboolean(L, 1);
	return 1;
}

static int input_wait(lua_State *L)
{
	SDL_Event event = {0};
	SDL_Event *next = NULL;

	if ( lua_istable(L, 1) ) {
		next = &event;
	}

	/* Return the event if a capture was requested */
	if ( SDL_WaitEvent(next) && next ) {
		ConvertTableIntoEvent(L, next);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/**Initialization*************************************************************/

void SDLua_OpenInputModule(lua_State *L)
{
	const luaL_reg kInterface[] = {
		{"push", input_push},
		{"wait", input_wait},
		{0}
	};
	SDLKey i;

	SDLua_PushPackage(L);
	luaL_register(L, NULL, kInterface);

	/* The queue function requires 1 upvalue. Instead of allocating a table at
	   each iteration of a for loop we use the table from the last iteration */
	lua_newtable(L);
	lua_pushcclosure(L, input_queue, 1);
	lua_setfield(L, -2, "queue");

	lua_pop(L, 1);

	lua_newtable(L);
	hooks = luaL_ref(L, LUA_ENVIRONINDEX);

	/* Create a lookup table for converting keys from string to SDLKeys */
	lua_newtable(L);
	for ( i = SDLK_FIRST; i < SDLK_LAST; ++i ) {
		lua_pushstring(L, SDL_GetKeyName(i));
		lua_pushnumber(L, i);
		lua_rawset(L, -3);
	}
	keys = luaL_ref(L, LUA_ENVIRONINDEX);
}
