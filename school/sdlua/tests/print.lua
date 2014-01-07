
local SDLua = require "SDLua"
local event = {}
local screen = SDLua.openscreen(800, 600)

print(SDLua.getfont("default").from)
screen.font = SDLua.getfont("Verdana.ttf", 48)

screen.printc = function (surface, text)
  local w, h = SDLua.getfont("Verdana.ttf", 48):measure(text)
  local x = (surface.w - w) / 2
  local y = (surface.h - h) / 2
  return surface:print(x, y, text)
end

screen:printc("Hello Lua from SDLua! bV"):update()

repeat
  SDLua.wait(event)
until event.quit
