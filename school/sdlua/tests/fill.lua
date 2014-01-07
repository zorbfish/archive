
local SDLua = require "SDLua"
local color = {}
local area = {}

local screen = SDLua.openscreen(800, 600, 24, "fullscreen", "accelerate")
screen:fill():update()

repeat
  for event in SDLua.queue do
    if event.pressed == "escape" then return end
  end

  color[1] = math.random(0, 255)
  color[2] = math.random(0, 255)
  color[3] = math.random(0, 255)

  area[1] = math.random(0, screen.w)
  area[2] = math.random(0, screen.h)
  area[3] = math.random(0, 50)
  area[4] = math.random(0, 50)

  screen:fill(color, area):update(area)
until finished