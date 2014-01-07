#ifndef image_h
#define image_h

#include "lua.h"
#include "blank.h"

/* Extends the functionality of the video module's getsurface function */
extern void SDLua_OpenImageModule(lua_State *L, SDLua_Factory surfaceFactory);

#endif
