#include "font.h"
#include "lua.h"
#include "SDL.h"
#include "blank.h"
#include "SDLua.h"

/* Foward declare the font structure to avoid the inclusion of SDL_ttf.h */
typedef struct TTF_Font *Font;

/* The the file name of the default font loaded by the font extension module
   The font is included as part of the distribution of the SDLua package */
static const char *kDefaultFont = "Arial.ttf";

typedef struct {
	void *object;
	int (*Init)();
	void (*Quit)();
	Font (*OpenFont)(const char *, int);
	void (*CloseFont)(Font);
	SDL_Surface *(*RenderText_Shaded)(Font,
		const char *, SDL_Color, SDL_Color);
	int (*SizeText)(Font, const char *, int *, int *);
} SDL_TTF;

static int SO = LUA_REFNIL;
static SDLua_Factory fontFactory = 0;

/**Private********************************************************************/

static Font GetFont(lua_State *L, int index)
{
	return *(Font *)lua_touserdata(L, index);
}

static SDL_TTF *GetFontLib(lua_State *L)
{
	SDL_TTF *font;
	lua_rawgeti(L, LUA_ENVIRONINDEX, SO);
	font = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return font;
}

static int CloseFont(lua_State *L)
{
	SDLua_CloseFactory(L, fontFactory);
	SDL_UnloadObject(GetFontLib(L)->object);
	return 0;
}

/**Public*********************************************************************/

static int video_getfont(lua_State *L)
{
	Font *font;
	
	/* Check if a request for the loaded default font was made */
	lua_pushliteral(L, "default");
	if ( lua_equal(L, -1, 1) ) {
		lua_pushvalue(L, lua_upvalueindex(1));
		return 1;
	}

	font = SDLua_CreateResource(L, fontFactory);
	if ( !font ) {
		return SDLua_Error(L);
	}

	*font = GetFontLib(L)->OpenFont(luaL_checkstring(L, 1),
		(int)luaL_checknumber(L, 2));
	if ( !*font ) {
		return SDLua_Error(L);
	}

	return 1;
}

static int video_print(lua_State *L)
{
	SDL_Surface *text = NULL;
	SDL_Rect printto = {0};
	static SDL_Color white = {255,255,255};
	static SDL_Color black = {0};

	/* If the surface has a font attached to it use it to print rather than the
	   default font */
	lua_getfield(L, 1, "font");
	if ( lua_isnil(L, -1) ) {
		lua_pushvalue(L, lua_upvalueindex(1));
	}

	text = GetFontLib(L)->RenderText_Shaded(GetFont(L, -1),
		luaL_checkstring(L, 4), white, black);

	/* Gather position information */
	printto.x = (Sint16)lua_tonumber(L, 2) - 1;
	printto.y = (Sint16)lua_tonumber(L, 3) - 1;
	printto.w = (Uint16)text->w;
	printto.h = (Uint16)text->h;

	SDL_BlitSurface(text, NULL, *(SDL_Surface **)lua_touserdata(L, 1), &printto);
	SDL_FreeSurface(text);

	return SDLua_Self(L);
}

static int font_dispose(lua_State *L)
{
	Font font = GetFont(L, 1);
	if ( font ) {
		GetFontLib(L)->CloseFont(font);
		*(Font *)lua_touserdata(L, 1) = NULL;
		printf("freed font\n");
	}
	return 0;
}

static int font_measure(lua_State *L)
{
	int w, h;
	GetFontLib(L)->SizeText(GetFont(L, 1), luaL_checkstring(L, 2), &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

/**Initialization*************************************************************/

void SDLua_OpenFontModule(lua_State *L, SDLua_Factory surfaceFactory)
{
	const luaL_reg kFontInterface[] = {
		{"dispose", font_dispose},
		{"__gc", font_dispose},
		{"measure", font_measure},
		{0}
	};
	Font *defFont = NULL;
	SDL_TTF *font = lua_newuserdata(L, sizeof(SDL_TTF));
	font->object = SDL_LoadObject("SDL_ttf");
	if ( !font->object ) {
		lua_pop(L, 1);
		return;
	}
	SO = luaL_ref(L, LUA_ENVIRONINDEX);

	/* Load the functions needed */
	font->Init = (int (*)())(ptrdiff_t)
		SDL_LoadFunction(font->object, "TTF_Init");
	font->Quit = (void (*)())(ptrdiff_t)
		SDL_LoadFunction(font->object, "TTF_Quit");
	font->OpenFont = (Font (*)(const char *, int))(ptrdiff_t)
		SDL_LoadFunction(font->object, "TTF_OpenFont");
	font->CloseFont = (void (*)(Font))(ptrdiff_t)
		SDL_LoadFunction(font->object, "TTF_CloseFont");
	font->RenderText_Shaded = (SDL_Surface *(*)(Font, const char *,
		SDL_Color, SDL_Color))(ptrdiff_t)SDL_LoadFunction(font->object,
		"TTF_RenderText_Shaded");
	font->SizeText = (int (*)(Font, const char *, int*, int *))(ptrdiff_t)
		SDL_LoadFunction(font->object, "TTF_SizeText");

	/* Initialize the library and clean up if it fails */
	if ( font->Init() < 0 ) {
		SDL_UnloadObject(font->object);
		lua_pop(L, 1);
		return;
	}

	fontFactory = SDLua_CreateFactory(L, sizeof(Font *), kFontInterface);
	
	/* Load the default font and clean up if it fails */
	defFont = SDLua_CreateResource(L, fontFactory);
	*defFont = font->OpenFont(kDefaultFont, 16);
	if ( !*defFont ) {
		font->Quit();
		SDL_UnloadObject(font->object);
		lua_pop(L, 2);
		return;
	}
	lua_pushstring(L, kDefaultFont);
	lua_setfield(L, -2, "from");

	/* Add getfont to the package */
	SDLua_PushPackage(L);
	lua_insert(L, -2);
	lua_pushvalue(L, -1);
	lua_pushcclosure(L, video_getfont, 1);
	lua_setfield(L, -3, "getfont");

	/* Add print to the package and surface metatable */
	lua_pushcclosure(L, video_print, 1);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "print");
	lua_remove(L, -2);

	SDLua_GetResourceMetatable(L, surfaceFactory);
	lua_insert(L, -2);
	lua_setfield(L, -2, "print");
	lua_pop(L, 1);

	SDLua_OnExit(L, CloseFont, 0);
}
