
local SDLua = require "SDL"
local event = {}

SDLua.openscreen()

local sfx = SDLua.getsound("example.wav"):play()

SDLua.delay(1)
sfx:stop()

repeat
  SDLua.wait(event)
until event.soundends == sfx

collectgarbage("collect")