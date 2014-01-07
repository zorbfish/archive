
local SDLua = require "SDLua"

local screen = SDLua.openscreen(148, 148)
assert(screen)
assert(SDLua.screen, "SDLua.screen not found")
assert(screen == SDLua.screen, "SDLua.screen not equal to return")

print(screen.w, screen.h)

screen = SDLua.openscreen(320, 240, "fullscreen")
assert(screen, "Fullscreen test failed")

screen = SDLua.openscreen(640, 480, "accelerate")
assert(screen, "Accelerated test failed")

print "Please resize the application window"

screen = SDLua.openscreen(320, 240, "resize")
assert(screen, "Resize test failed")

local event = {}
repeat
  SDLua.wait(event)
until event.size
