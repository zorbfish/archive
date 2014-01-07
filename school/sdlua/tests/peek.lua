
local SDL = require "SDL"
local event = {}

local screen = SDL.openscreen(148, 148)
local logo = SDL.getsurface("logo.gif")

screen:fill({255, 255, 255})
logo:blit(10, 10).update(screen)

print"Click on the Lua logo to get a color"

repeat
  SDL.wait(event)

  if event.pressed == "mouse" then
    print(string.format("RGB at (%d, %d): %d, %d, %d",
      event.x, event.y, logo:peek(event.x, event.y)))
  end
until event.quit
