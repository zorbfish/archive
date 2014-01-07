#ifndef audio_h
#define audio_h

#include "lua.h"

/* Opens the mixer driver and adds functions for playing music and sounds */
extern void SDLua_OpenAudioModule(lua_State *L);

#endif
