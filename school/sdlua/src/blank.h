#ifndef blank_h
#define blank_h

#include "lauxlib.h"
#include "lua.h"
#include "SDL_error.h"

/* Resets the stack top to the self argument and returns 1 */
extern int SDLua_Self(lua_State *L);

/**Errors*********************************************************************/

/* Pipe errors into SDL */
#define SDLua_SetError SDL_SetError

/* Pushes nil and the error string returned by SDL and the returns 2 */
extern int SDLua_Error(lua_State *L);

/* Pushes the error reported by SDL/SDLua, calls lua_error, and returns 1. The
   function is designed for non-recoverable errors that cannot be handled */
extern int SDLua_Fail(lua_State *L);

/**Resources******************************************************************/

typedef int SDLua_Factory;

/* Creates a factory for creating a new types of resources out of userdata
   The size of the resource and the metatable on the userdata is required */
extern SDLua_Factory SDLua_CreateFactory(lua_State *L, size_t size,
																				 const luaL_reg metatable[]);

/* Pushes the metatable created with the factory onto the stack. Returns 1
   if the factory has a metatable associated with it or 0 and pushes nothing */
extern int SDLua_GetResourceMetatable(lua_State *L, SDLua_Factory factory);

/* Allocates a new userdata of the resource type specified by the factory
   and attaches the metatable associated with it (if any). Returns the
	 userdata on the stack as well as a raw pointer to it */
extern void *SDLua_CreateResource(lua_State *L, SDLua_Factory factory);

/* Closes a factory. All non-collected resources are now automatically gc'ed
   by the factory calling each one's dispose function */
extern void SDLua_CloseFactory(lua_State *L, SDLua_Factory factory);

#endif
