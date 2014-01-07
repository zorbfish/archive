
local SDL = require "SDL"

local screen = assert(SDL.openscreen(800, 600))
local logo=SDL.getsurface("logo.gif")
local x,y = 0,0
local dirx, diry

repeat
  local j=SDL.poll"joystick"

  if j.x > -30000 and j.x < 30000 then
    if j.x > 5000 then dirx = "right"
    elseif j.x < -5000 then dirx = "left"
    else dirx = nil end
  end
  if j.y > -30000 and j.y < 30000 then
    if j.y > 5000 then diry = "down"
    elseif j.y < -5000 then diry = "up"
    else diry = nil end
  end

  if dirx == "left" then x = x - 2 end
  if dirx == "right" then x = x + 2 end
  if diry == "up" then y = y - 2 end
  if diry == "down" then y = y + 2 end

  SDL.screen:fill()
  logo:blit(x,y).update(screen)
until finished