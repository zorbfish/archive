#include "audio.h"
#include "lauxlib.h"
#include "lua.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "blank.h"
#include "input.h"
#include "SDLua.h"

static int musicEndsHook = 0;
static SDLua_Factory musicFactory = 0;
static SDLua_Factory soundFactory = 0;

/**Private********************************************************************/

static int CloseAudio(lua_State *L)
{
	SDLua_CloseFactory(L, musicFactory);
	SDLua_CloseFactory(L, soundFactory);
	Mix_CloseAudio();
	return 0;
}

/* Redirects the mixer's music end hook to push a custom event ... */
static void MixerHook_OnMusicEnd(void)
{
	SDL_Event event = {0};
	event.type = SDL_USEREVENT;
	event.user.code = musicEndsHook;
	SDL_PushEvent(&event);
}

/* ... which is then caught here and converted into a musicends event */
static int SDLuaHook_OnMusicEnd(lua_State *L)
{
	/* The table to convert is on top */
	lua_pushliteral(L, "musicends");
	lua_pushboolean(L, 1);
	lua_rawset(L, -3);
	return 0;
}

/* Handy getters for the Mixer structures */

static Mix_Music *GetMusic(lua_State *L, int index)
{
	return *(Mix_Music **)lua_touserdata(L, index);
}

static Mix_Chunk *GetSound(lua_State *L, int index)
{
	return *(Mix_Chunk **)lua_touserdata(L, index);
}

/**Public*********************************************************************/

static int audio_getmusic(lua_State *L)
{
	Mix_Music **music = SDLua_CreateResource(L, musicFactory);
	if ( !music ) {
		return SDLua_Error(L);
	}
	
	*music = Mix_LoadMUS(luaL_checkstring(L, 1));
	if ( !*music ) {
		return SDLua_Error(L);
	}

	return 1;
}

static int audio_getsound(lua_State *L)
{
	Mix_Chunk **sound = SDLua_CreateResource(L, soundFactory);
	if ( !sound ) {
		return SDLua_Error(L);
	}
	
	*sound = Mix_LoadWAV(luaL_checkstring(L, 1));
	if ( !*sound ) {
		return SDLua_Error(L);
	}

	return 1;
}

/* Does not need to differentiate between a sound and music resource */
static int audio_play(lua_State *L)
{
	if ( lua_isuserdata(L, 1) ) {
		lua_getfield(L, 1, "play");
		lua_insert(L, 1);
		lua_call(L, lua_gettop(L) - 1, 1);
	}
	return 1;
}

static int music_play(lua_State *L)
{
	int loop = -1;
	if ( lua_isnumber(L, 2) ) {
		loop = (int)lua_tonumber(L, 2);
	}

	Mix_PlayMusic(GetMusic(L, 1), loop);

	return SDLua_Self(L);
}

static int sound_play(lua_State *L)
{
	int loop = -1;
	if ( lua_isnumber(L, 2) ) {
		loop = loop + (int)lua_tonumber(L, 2);
		if ( loop < 0 ) {
			return SDLua_Self(L);
		}
	}

	Mix_PlayChannel(-1, GetSound(L, 1), loop);

	return SDLua_Self(L);
}

static int music_dispose(lua_State *L)
{
	Mix_Music *music = GetMusic(L, 1);
	if ( music ) {
		Mix_FreeMusic(music);
		*(Mix_Music **)lua_touserdata(L, 1) = NULL;
		printf("freed music\n");
	}
	return 0;
}

static int sound_dispose(lua_State *L)
{
	Mix_Chunk *sound = GetSound(L, 1);
	if ( sound ) {
		Mix_FreeChunk(sound);
		*(Mix_Chunk **)lua_touserdata(L, 1) = NULL;
		printf("freed sound\n");
	}
	return 0;
}

/**Initialization*************************************************************/

void SDLua_OpenAudioModule(lua_State *L)
{
	const luaL_reg kInterface[] = {
		{"getmusic", audio_getmusic},
		{"getsound", audio_getsound},
		{"play", audio_play},
		{0}
	};
	const luaL_reg kMusicInterface[] = {
		{"play", music_play},
		{"dispose", music_dispose},
		{"__gc", music_dispose},
		{0}
	};
	const luaL_reg kSoundInterface[] = {
		{"play", sound_play},
		{"dispose", sound_dispose},
		{"__gc", sound_dispose},
		{0}
	};

	if ( Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < -1 ) {
		SDLua_Fail(L);
	}

	SDLua_PushPackage(L);
	luaL_register(L, NULL, kInterface);
	lua_pop(L, 1);

	musicFactory = SDLua_CreateFactory(L, sizeof(Mix_Music **), kMusicInterface);
	soundFactory = SDLua_CreateFactory(L, sizeof(Mix_Chunk **), kSoundInterface);
	
	/* Create an event for the audio module. When the currently playing
	   music clip ends a "musicends" event will be enqueued */
	musicEndsHook = SDLua_SetCustomEventHook(L, SDLuaHook_OnMusicEnd);
	Mix_HookMusicFinished(MixerHook_OnMusicEnd);

	SDLua_OnExit(L, CloseAudio, 0);
}
