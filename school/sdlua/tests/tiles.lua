
local SDL = require "SDL"

local screen = assert(SDL.openscreen(320, 240))
local tiles = assert(SDL.getsurface "tiles.png")

tiles[1] = { 0, 0, 32, 32}
tiles[2] = {32, 0, 64, 32}
tiles[3] = {64, 0, 96, 32}
tiles[4] = {32, 32, 64, 64}

local fringe = {
 [132] = 4
}

local ground = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
}

local camwidth = 10
local camheight = 10
local x, y = 0, 0

screen:clip({0, 0, 320, 240})
repeat
  for event in SDL.queue do
    if event.quit or event.pressed == "escape" then return end
    if event.pressed == "right" then x = x + 10 end
    if event.pressed == "left" then x = x -  10 end
    if event.pressed == "up" then y = y -  10 end
    if event.pressed == "down" then y = y +  10 end
  end

  if x < 0 then
    x = 0
  end
  if y < 0 then
    y = 0
  end
  if x >= (19 * 32) then
    x = 19 * 32
  end
  if y >= (14 * 32) then
    y = 14 * 32
  end

  local xmod = x % 32
  local ymod = y % 32
  local xstart = math.floor(x / 32)
  if xstart >= 19 then xstart = 19 end
  local ystart = math.floor(y / 32)
  if ystart >= 14 then ystart = 14 end

  local xend = camwidth + xstart
  --if xmod ~= 0 then xend = xend + 1 end
  local yend = camheight + ystart
  --if ymod ~= 0 then yend = yend + 1 end
 
  screen:fill()

  local xx, yy = 0, -ymod
  for j = ystart, yend do
    xx = -xmod
    for i = xstart, xend do
      tiles:blit(xx, yy, screen, tiles[ground[(j + 1) * 20 + (i + 1)]])
      local z = fringe[(j + 1) * 20 + (i + 1)]
      if z ~= nil then
        tiles:blit(xx, yy, screen, tiles[z])
      end
      xx = xx + 32
    end
    yy = yy + 32
  end

  screen:update()
until finished
