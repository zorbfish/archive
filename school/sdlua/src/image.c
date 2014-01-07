#include "image.h"
#include "lua.h"
#include "SDL.h"
#include "blank.h"
#include "SDLua.h"

typedef struct {
	void *object;

	/* Holds IMG_Load */
	SDL_Surface *(*Load)(const char *);
} SDL_image;

static int SO = LUA_REFNIL;

/**Private********************************************************************/

static int UnloadSO(lua_State *L)
{
	lua_rawgeti(L, LUA_ENVIRONINDEX, SO);
	SDL_UnloadObject(((SDL_image *)lua_touserdata(L, -1))->object);
	return 0;
}

/**Public*********************************************************************/

static int video_getsurfaceEXT(lua_State *L)
{
	SDL_image *image = NULL;
	SDL_Surface **surface = SDLua_CreateResource(L,
		(SDLua_Factory)lua_tonumber(L, lua_upvalueindex(1)));
	if ( !surface ) {
		return SDLua_Error(L);
	}
	
	image = lua_touserdata(L, lua_upvalueindex(2));
	*surface = image->Load(luaL_checkstring(L, 1));
	if ( !surface ) {
		return SDLua_Error(L);
	}
	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "from");

	lua_pushnumber(L, (*surface)->w);
	lua_setfield(L, -2, "w");
	lua_pushnumber(L, (*surface)->h);
	lua_setfield(L, -2, "h");

	return 1;
}

/**Initialization*************************************************************/

void SDLua_OpenImageModule(lua_State *L, SDLua_Factory surfaceFactory)
{
	SDL_image *image = lua_newuserdata(L, sizeof(SDL_image));
	image->object = SDL_LoadObject("SDL_image");
	if ( !image->object ) {
		lua_pop(L, 1);
		return;
	}
	SO = luaL_ref(L, LUA_ENVIRONINDEX);

	/* Load the function and store the shared object in the environment */
	image->Load = (SDL_Surface *(*)(const char*))(ptrdiff_t)
		SDL_LoadFunction(image->object, "IMG_Load");

	/* Replace the core getsurface function with the extended version */
	SDLua_PushPackage(L);
	lua_pushliteral(L, "getsurface");
	lua_pushnumber(L, surfaceFactory);
	lua_rawgeti(L, LUA_ENVIRONINDEX, SO);
	lua_pushcclosure(L, video_getsurfaceEXT, 2);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	SDLua_OnExit(L, UnloadSO, 0);
}
