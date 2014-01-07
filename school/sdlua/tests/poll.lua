
local SDLua = require "SDLua"
local pressed

SDLua.openscreen().caption("Press escape to quit")

repeat
  pressed = SDLua.poll"keyboard"
until pressed.escape