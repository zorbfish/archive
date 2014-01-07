#ifndef video_h
#define video_h

#include "lua.h"

/* Opens the video driver and adds functions for 2D software rendering */
extern void SDLua_OpenVideoModule(lua_State *L);

#endif
