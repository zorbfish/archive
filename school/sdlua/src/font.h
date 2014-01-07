#ifndef font_h
#define font_h

#include "lua.h"
#include "blank.h"

/* Extends the video module to support TrueType font rendering */
extern void SDLua_OpenFontModule(lua_State *L, SDLua_Factory surfaceFactory);

#endif
