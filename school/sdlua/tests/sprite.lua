
local SDLua = require "SDL"

local setmetatable = setmetatable
local rawset = rawset
local pairs = pairs
local next = next
local unpack = unpack
local print = print
local rawget = rawget

module "sprite"

sprites = setmetatable({}, {__mode = "k"})

local sprite = {}
function sprite.__newindex(t, k, v)
  if k == "x" then
    rawset(t, "dirty", true)
    rawset(t.rect, 1, v)
  elseif k == "y" then
    rawset(t, "dirty", true)
    rawset(t.rect, 2, v)
  elseif k == "w" then
    rawset(t, "dirty", true)
    rawset(t.rect, 3, v)
  elseif k == "h" then
    rawset(t, "dirty", true)
    rawset(t.rect, 4, v)
  else
    rawset(t, k, v)
  end
end

function sprite.hits(a, b)
  local left, right, up, down
  local old = {unpack(a.rect)}

  if not (((a.x + a.w) >= b.x) and (a.x <= (b.x + b.w))
    and ((a.y + a.h) >= b.y) and (a.y <= (b.y + b.h))) then
    return
  end
  if (old[1] <= b.x)
    and (b.x <= (old[1] + old[3]))
    and ((old[1] + old[3]) <= (b.x + b.w)) then
    a.x = b.x - a.w
    left = true
  end
  if (b.x <= old[1])
    and (old[1] <= (b.x + b.w))
    and ((b.x + b.w) <= (old[1] + old[3])) then
    a.x = b.x + b.w
    right = true
  end
  if (old[2] <= b.y)
    and (b.y <= (old[2] + old[4]))
    and ((old[2] + old[4]) <= (b.y + b.h)) then
    a.y = b.y - a.h
    up = true
  end
  if (b.y <= old[2])
    and (old[2] <= (b.y + b.h))
    and ((b.y + b.h) <= (old[2] + old[4])) then
    a.y = b.y + b.h
    down = true
  end

  if not (left == right) then
    a.xdir = -a.xdir
  end
  if not (up == down) then
    a.ydir = -a.ydir
  end

  return left or right or up or down
end

function hits(a, b)
  a.rect = {a.x, a.y, a.w, a.h}
  a.xdir, a.ydir = 1, 1
  return sprite.hits(a, b)
end

function sprite.__index(t, k)
  if k == "x" then
    return rawget(t.rect, 1)
  elseif k == "y" then
    return rawget(t.rect, 2)
  elseif k == "w" then
    return rawget(t.rect, 3)
  elseif k == "h" then
    return rawget(t.rect, 4)  
  end
  return rawget(t, k) or rawget(sprite, k)
end

local function dup(t)
  local r = {}
  for k, v in pairs(t) do r[k] = v end
  return r
end

function new(self, x, y, width, height, color)
  local t = {
    color = color,
    rect = {x, y, width, height},
    dirty = true
  }
  self.sprites[t] = dup(t.rect)
  return setmetatable(t, sprite)
end

function draw(self)
  local dirty = {}

  for sprite, rect in pairs(self.sprites) do
    if sprite.dirty then
      -- Clear the old position, update the new position
      SDLua.screen:fill({}, rect):fill(sprite.color, sprite.rect)

      -- Save the new position as its old position
      self.sprites[sprite] = dup(sprite.rect)

      -- Sprite is updated
      sprite.dirty = false

      -- Add both rectangles for update
      dirty[#dirty + 1] = rect
      dirty[#dirty + 1] = sprite.rect
    end
  end

  -- Update if there are rectangles
  if next(dirty) ~= nil then
    SDLua.screen:update(unpack(dirty))
  end
end
