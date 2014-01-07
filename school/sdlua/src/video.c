#include "video.h"
#include "lauxlib.h"
#include "lua.h"
#include "SDL.h"
#include "blank.h"
#include "image.h"
#include "font.h"

#include "SDLua.h"

static SDLua_Factory surfaceFactory = 0;

/**Private********************************************************************/

/* Collects the arguments for the video mode and returns the screen */
static SDL_Surface *InitScreen(lua_State *L)
{
	/* The default screen state opens a centered window with a size of half the
	   the current screen resolution */
	int w = SDL_GetVideoInfo()->current_w / 2;
	int h = SDL_GetVideoInfo()->current_h / 2;
	int bpp = 0;
	Uint32 flags = 0;

	/* Maps the SDLua string tokens onto their SDL screen flag equivalents */
	struct {
		const char *name;
		Uint32 flag;
	} kMap[] = {
		{"fullscreen",	SDL_FULLSCREEN},
		{"accelerate",	SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWPALETTE},
		{"opengl",			SDL_OPENGL},
		{"resize",			SDL_RESIZABLE},
		{0}
	};

	if ( lua_gettop(L) > 0 ) {
		/* Optional: the width, height, and bit depth */
		if ( lua_isnumber(L, 1) ) {
			w = (int)lua_tonumber(L, 1);
		}
		if ( lua_isnumber(L, 2) ) {
			h = (int)lua_tonumber(L, 2);
		}
		if ( lua_isnumber(L, 3) ) {
			bpp = (int)lua_tonumber(L, 3);
		}

		/* openscreen may recieve any of the mapped string tokens in any order */
		while ( lua_gettop(L) > 0 ) {
			int i;
			if ( lua_isstring(L, -1) ) {	
				for ( i = 0; kMap[i].name; ++i ) {
					if ( !SDL_strcmp(kMap[i].name, lua_tostring(L, -1)) ) {
						flags |= kMap[i].flag;
						break;
					}
				}
			}
			lua_pop(L, 1);
		}
	}

	SDL_putenv("SDL_VIDEO_CENTERED=1");
	return SDL_SetVideoMode(w, h, bpp, flags);
}

static int CloseVideo(lua_State *L)
{
	SDLua_CloseFactory(L, surfaceFactory);
	return 0;
}

static SDL_Surface *GetSurface(lua_State *L, int index)
{
	return *(SDL_Surface **)lua_touserdata(L, index);
}

static SDL_Rect SurfaceToRect(SDL_Surface *surface)
{
	SDL_Rect r = {0};
	r.w = (Uint16)surface->w;
	r.h = (Uint16)surface->h;
	return r;
}

/**Public*********************************************************************/

static int video_getsurface(lua_State *L)
{
	SDL_Surface **surface = SDLua_CreateResource(L, surfaceFactory);
	if ( !surface ) {
		return SDLua_Error(L);
	}
	
	*surface = SDL_LoadBMP(luaL_checkstring(L, 1));
	if ( !*surface ) {
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

static int screen_caption(lua_State *L)
{
	SDL_WM_SetCaption(luaL_checkstring(L, 1), NULL);
	return 0;
}

static int screen_showcursor(lua_State *L)
{
	SDL_ShowCursor(lua_toboolean(L, 1));
	return 0;
}

static int screen_grabinput(lua_State *L)
{
	L;
	SDL_WM_GrabInput(SDL_GRAB_ON);
	return 0;
}

static int video_openscreen(lua_State *L)
{
	SDL_Surface *screen = InitScreen(L);
	if ( !screen ) {
		return SDLua_Error(L);
	}

	/* Check if the screen already exists */
	SDLua_PushPackage(L);
	lua_getfield(L, -1, "screen");
	if ( lua_isnil(L, -1) ) {
		/* A surface resource is not created for the screen surface because SDL owns
			that memory. Instead the screen userdata needs to be created manually */
		*(SDL_Surface **)lua_newuserdata(L, sizeof(SDL_Surface **)) = screen;

		/* Attach the surface metatable onto the screen userdata. A few modifications
			are made such as stripping off the garbage collection functions and adding
			some functions for the window manager (if available). Because of this a
			copy of the surface metatable needs to be used instead of the original. */
		SDLua_GetResourceMetatable(L, surfaceFactory);
		lua_newtable(L);
		for ( lua_pushnil(L); lua_next(L, -3); lua_pop(L, 0) ) {
			lua_pushvalue(L, -2);
			lua_insert(L, -2);
			lua_rawset(L, -4);
		}
		lua_remove(L, -2);
		lua_pushnil(L);
		lua_setfield(L, -2, "dispose");
		lua_pushnil(L);
		lua_setfield(L, -2, "__gc");
		lua_setmetatable(L, -2);
		lua_newtable(L);
		lua_setfenv(L, -2);
		if ( SDL_GetVideoInfo()->wm_available ) {
			lua_pushcfunction(L, screen_showcursor);
			lua_setfield(L, -2, "cursor");
			lua_pushcfunction(L, screen_grabinput);
			lua_setfield(L, -2, "grabinput");
			lua_pushcfunction(L, screen_caption);
			lua_setfield(L, -2, "caption");
		}

		/* Keep the screen handy in the package */
		SDLua_PushPackage(L);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "screen");
		lua_pop(L, 1);
	} else {
		/* The old screen userdata is already setup so reuse it but change the
		   internal surface pointer it boxes. Users can set/get data on the screen
		   without worry */
		*(SDL_Surface **)lua_touserdata(L, -1) = screen;
	}

	lua_pushnumber(L, screen->w);
	lua_setfield(L, -2, "w");
	lua_pushnumber(L, screen->h);
	lua_setfield(L, -2, "h");

	if ( SDL_GetVideoInfo()->wm_available ) {
		/* The standard Lua interpreter packs arguments from the command line into
		   a global table called arg. We can get the name of the script using it */
		lua_getglobal(L, "arg");
		if ( lua_istable(L, -1 ) ) {
			lua_rawgeti(L, -1, 0);
			SDL_WM_SetCaption(lua_tostring(L, -1), NULL);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	return 1;
}

static int surface_blit(lua_State *L)
{
	SDL_Surface *self = GetSurface(L, 1);
	SDL_Surface *target = SDL_GetVideoSurface();
	SDL_Rect from = SurfaceToRect(self);
	SDL_Rect to = from;

	/* Arguments are collected backwards. It makes optional arguments easier */
	switch ( lua_gettop(L) ) {
		case 5:
			lua_rawgeti(L, 5, 1);
			from.x = (Sint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 5, 2);
			from.y = (Sint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 5, 3);
			from.w = (Uint16)lua_tonumber(L, -1);
			lua_rawgeti(L, 5, 4);
			from.h = (Uint16)lua_tonumber(L, -1);
			lua_pop(L, 4);
		case 4:
			target = GetSurface(L, 4);
		default:
			to.x = (Sint16)lua_tonumber(L, 2);
			to.y = (Sint16)lua_tonumber(L, 3);
			break;
	}
	SDL_BlitSurface(self, &from, target, &to);

	return SDLua_Self(L);
}

static int surface_clip(lua_State *L)
{
	SDL_Rect old;
	SDL_GetClipRect(GetSurface(L, 1), &old);

	if ( lua_istable(L, 2) ) {
		SDL_Rect clip;
		lua_rawgeti(L, 2, 1);
		lua_rawgeti(L, 2, 2);
		lua_rawgeti(L, 2, 3);
		lua_rawgeti(L, 2, 4);
		clip.x = (Sint16)lua_tonumber(L, -4);
		clip.y = (Sint16)lua_tonumber(L, -3);
		clip.w = (Uint16)lua_tonumber(L, -2);
		clip.h = (Uint16)lua_tonumber(L, -1);
		SDL_SetClipRect(GetSurface(L, 1), &clip);
	}

	lua_newtable(L);
	lua_pushnumber(L, old.x);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, old.y);
	lua_rawseti(L, -2, 2);
	lua_pushnumber(L, old.w);
	lua_rawseti(L, -2, 3);
	lua_pushnumber(L, old.h);
	lua_rawseti(L, -2, 4);

	return 1;
}

static int surface_dispose(lua_State *L)
{
	SDL_Surface *surface = GetSurface(L, 1);
	if ( surface ) {
		SDL_FreeSurface(surface);
		*(SDL_Surface **)lua_touserdata(L, 1) = NULL;
		printf("freed surface\n");
	}
	return 0;
}

static int surface_dump(lua_State *L)
{
	if ( SDL_SaveBMP(GetSurface(L, 1), luaL_checkstring(L, 2)) ) {
		return SDLua_Error(L);
	}
	return SDLua_Self(L);
}

static int surface_fill(lua_State *L)
{
	SDL_Rect area = {0};
	SDL_Rect *fill = NULL;
	Uint32 color = 0;

	switch ( lua_gettop(L) ) {
		case 3:
			lua_rawgeti(L, 3, 1);
			area.x = (Sint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 3, 2);
			area.y = (Sint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 3, 3);
			area.w = (Uint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 3, 4);
			area.h = (Uint16)lua_tonumber(L, -1) - 1;
			lua_pop(L, 4);
			fill = &area;
		case 2:
			lua_rawgeti(L, 2, 1);
			lua_rawgeti(L, 2, 2);
			lua_rawgeti(L, 2, 3);
			color = SDL_MapRGB(GetSurface(L, 1)->format,
				(Uint8)lua_tonumber(L, -3), (Uint8)lua_tonumber(L, -2),
				(Uint8)lua_tonumber(L, -1));
			lua_pop(L, 3);
		default:
			break;
	}
	SDL_FillRect(GetSurface(L, 1), fill, color);

	return SDLua_Self(L);
}

static int surface_keyout(lua_State *L)
{
	Uint32 colorkey = 0;

	if ( lua_gettop(L) > 1 ) {
		lua_rawgeti(L, 2, 1);
		lua_rawgeti(L, 2, 2);
		lua_rawgeti(L, 2, 3);
	} else {
		lua_getfield(L, 1, "peek");
		lua_pushvalue(L, 1);
		lua_pushnumber(L, 1);
		lua_pushnumber(L, 1);
		lua_call(L, 3, 3);
	}

	colorkey = SDL_MapRGB(GetSurface(L, 1)->format, (Uint8)lua_tonumber(L, -3),
		(Uint8)lua_tonumber(L, -2), (Uint8)lua_tonumber(L, -1));

	SDL_SetColorKey(GetSurface(L, 1), SDL_SRCCOLORKEY, colorkey);

	return SDLua_Self(L);
}

static int surface_peek(lua_State *L)
{
	SDL_Surface *surface = GetSurface(L, 1);
	if ( surface->format->BitsPerPixel != 8 ) {
		Uint32 *pixels = (Uint32 *)surface->pixels;
		Uint32 temp = 0;
		SDL_PixelFormat *fmt = surface->format;
		Uint32 pixel;

		pixel = pixels[((int)lua_tonumber(L, 3) - 1) *
			(surface->pitch * sizeof(Uint32)) + ((int)lua_tonumber(L, 2) - 1)];

		temp = pixel & fmt->Rmask;
		temp = temp >> fmt->Rshift;
		temp = temp << fmt->Rloss;
		lua_pushnumber(L, temp);

		temp = pixel & fmt->Gmask;
		temp = temp >> fmt->Gshift;
		temp = temp << fmt->Gloss;
		lua_pushnumber(L, temp);

		temp = pixel & fmt->Bmask;
		temp = temp >> fmt->Bshift;
		temp = temp << fmt->Bloss;
		lua_pushnumber(L, temp);
	} else {
		Uint8 *pixels = (Uint8 *)surface->pixels;
		Uint8 index = pixels[((int)lua_tonumber(L, 3) - 1) *
			(surface->pitch) + ((int)lua_tonumber(L, 2) - 1)];
		SDL_Color color = surface->format->palette->colors[index];
		lua_pushnumber(L, color.r);
		lua_pushnumber(L, color.g);
		lua_pushnumber(L, color.b);
	}
	return 3;
}

static int surface_update(lua_State *L)
{
	SDL_Rect rectlist[32];
	int i;

	switch ( lua_gettop(L) ) {
		case 1:
			SDL_Flip(GetSurface(L, 1));
			break;
		case 2:
			lua_rawgeti(L, 2, 1);
			rectlist[0].x = (Sint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 2, 2);
			rectlist[0].y = (Sint16)lua_tonumber(L, -1) - 1;
			lua_rawgeti(L, 2, 3);
			rectlist[0].w = (Uint16)lua_tonumber(L, -1);
			lua_rawgeti(L, 2, 4);
			rectlist[0].h = (Uint16)lua_tonumber(L, -1);
			SDL_UpdateRect(GetSurface(L, 1),
				rectlist[0].x, rectlist[0].y, rectlist[0].w, rectlist[0].h);
			break;
		default:
			i = 0;
			while ( lua_gettop(L) > 1 ) {
				lua_rawgeti(L, -1, 1);
				rectlist[i].x = (Sint16)lua_tonumber(L, -1) - 1;
				lua_rawgeti(L, -2, 2);
				rectlist[i].y = (Sint16)lua_tonumber(L, -1) - 1;
				lua_rawgeti(L, -3, 3);
				rectlist[i].w = (Uint16)lua_tonumber(L, -1);
				lua_rawgeti(L, -4, 4);
				rectlist[i].h = (Uint16)lua_tonumber(L, -1);
				lua_pop(L, 5);
				++i;
			}
			SDL_UpdateRects(GetSurface(L, 1), i, rectlist);
			break;
	}
	return 0;
}

/**Initialization*************************************************************/

void SDLua_OpenVideoModule(lua_State *L)
{
	const luaL_reg kInterface[] = {
		{"getsurface", video_getsurface},
		{"openscreen", video_openscreen},
		{0}
	};
	const luaL_reg kSurfaceInterface[] = {
		{"blit", surface_blit},
		{"clip", surface_clip},
		{"dispose", surface_dispose},
		{"dump", surface_dump},
		{"fill", surface_fill},
		{"keyout", surface_keyout},
		{"peek", surface_peek},
		{"__gc", surface_dispose},
		{"update", surface_update},
		{0}
	};

	SDLua_PushPackage(L);
	luaL_register(L, NULL, kInterface);
	lua_pop(L, 1);

	surfaceFactory = SDLua_CreateFactory(L, sizeof(SDL_Surface **), kSurfaceInterface);
	SDLua_OnExit(L, CloseVideo, 0);

	SDLua_OpenFontModule(L, surfaceFactory);
	SDLua_OpenImageModule(L, surfaceFactory);
}
