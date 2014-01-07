#ifndef input_h
#define input_h

#include "lua.h"

/* Adds functions for polling input devices and manipulating the event queue */
extern void SDLua_OpenInputModule(lua_State *L);

/* Hooks a Lua function to be called when unknown events are queued. It returns
   an integer that is to be used in the code field of a SDL_USEREVENT struct
	 
	 Hooks recieve two arguments. The first is a pointer to the SDL_Event struct
	 and the other is the Lua table that is to be converted into a SDLua event */
extern int SDLua_SetCustomEventHook(lua_State *L, lua_CFunction function);

#endif
