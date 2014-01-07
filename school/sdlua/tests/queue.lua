
local SDLua = require "SDLua"

SDLua.openscreen()
repeat
  SDLua.wait()
  for event in SDLua.queue do
    print(event)
    if event.quit then return end
  end
until finished