
local SDLua = require "SDLua"
local event = {}
local r, g, b
local x, y, i
local rand = math.random
local now, last

local screen = SDLua.openscreen(1600, 1200, 16, "fullscreen")
screen:fill():update()

SDLua.delay(1)
last = SDLua.getticks()
now = last

repeat
  i = i or 0

  for event in SDLua.queue do
    if event.quit or event.pressed then return end
  end

  r = rand(0, 255)
  g = rand(0, 255)
  b = rand(0, 255)

  x = rand(0, screen.w-1)
  y = rand(0, screen.h-1)

  screen:poke(x, y, r, g, b)
  i = i + 1

  if i > 50 then
    screen:update()
    i = 0
    now = SDLua.getticks()
    print("Pixels writes ", now - last)
    last = now
  end
until finished
