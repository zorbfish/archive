#!/usr/bin/env lua

local megasphere = assert(require"megasphere")
local SDL = assert(require"SDL")
local timer = require"timer"

--[[
  Hamburgers
--]]

local hamburger = {
  points = 20,
  velocity = 50 / timer.step,
  animated = true,
  frames = {
    { 0, 0, 32, 32},
    {32, 0, 64, 32}
  }
}
hamburger.__index = hamburger

function hamburger.tick(self)
  local maxx = SDL.screen.w + 2 * self.sprite.w

  for _,enemy in ipairs(self) do
    if enemy.alive then
      enemy.x = enemy.x + self.velocity

      enemy.animation.step()

      if enemy.x > maxx then
        enemy.x = -2 * self.sprite.w
      end
    end
  end
end

function hamburger.reset(self)
  local x = -self.sprite.w
  local y = 100
  local w = self.sprite.w
  local h = self.sprite.h

  local function next(o, frames)
    local n = o.animation.n
    n = n + 1
    if n > 2 then
      n = 1
    end

    o.animation.frame = frames[n]
    o.animation.n = n
  end

  local i = 1
  for c = 1, 14 do
    if (c % 2) == 0 then
      self[i].x = x
      self[i].y = y - 32
      self[i].w = w
      self[i].h = h
      self[i].alive = true

      local n = math.random(1, 2)
      self[i].animation = {
        n = n,
        frame = self.frames[n],
        step = timer.gettimer(math.random(10, 15), next, self[i], self.frames)
      }

      self[i+1].x = x
      self[i+1].y = y + 32
      self[i+1].w = w
      self[i+1].h = h
      self[i+1].alive = true

      local n = math.random(1, 2)
      self[i+1].animation = {
        n = n,
        frame = self.frames[n],
        step = timer.gettimer(math.random(10, 15), next, self[i+1], self.frames)
      }

      i = i + 2
    else
      self[i].x = x
      self[i].y = 100
      self[i].w = w
      self[i].h = h
      self[i].alive = true

      local n = math.random(1, 2)
      self[i].animation = {
        n = n,
        frame = self.frames[n],
        step = timer.gettimer(math.random(10, 15), next, self[i], self.frames)
      }
      i = i + 1
    end
    x = x - 64
  end
end

--[[
  Cookies
--]]

local c = {
  points = 30,
  reverse = 120
}

function c.tick()
  local down = descend()
  for enemy in wave do
    move(enemy, descend)
  end

  -- descend
  -- if moved offscreen wrap around
  -- move horizontally
  -- if moved offscreen wrap around
  -- reverse direction
end

--[[
  Bugs
--]]

local b = {
  points = 40
}

function b.tick()
  -- move right
  -- if moved offscreen wrap around
  -- descend
  -- if moved 1/4 from original position stop
  -- move right for awhile
  -- ascend to original position
end

--[[
  Tires
--]]

local t = {
  points = 50
}

function t.tick()
  -- descend
  -- if moved offscreen wrap around
  -- move horizontally
  -- if moved offscreen wrap around
  -- reverse direction
end

--[[
  Steam Irons
--]]

local i = {
  points = 60
}

function i.tick()
  -- descend
  -- move right
  -- move left
  -- descend
  -- fire bullet while descending
end

--[[
  Diamond Rings
--]]

local d = {
  points = 70
}

function d.tick()
  -- move right
  -- descend
end

--[[
  Bow Ties
--]]

local t = {
  points = 80
}

function t.tick()
  -- move up and down (sin curve)
  -- move right
end

--[[
  Space Dice (Lua)
--]]

local d = {
  points = 90
}

function d.tick()
  -- descend
  -- if moved offscreen wrap around
  -- pick random horizontal position
end

--[[
  Enemy
--]]

hamburger.next = hamburger

local enemy = {}

function enemy.init(self)
  hamburger.sprite = SDL.getsurface("data/burger.bmp"):keyout()

  for i = 1, 32 do
    self[i] = {
      x = 0,
      y = 0,
      alive = false
    }
  end

  setmetatable(self, hamburger):reset()
end

function enemy.draw(self, dt)
  for _,enemy in ipairs(self) do
    if enemy.alive then
      if self.animated then
        self.sprite:blit(enemy.x, enemy.y,
          SDL.screen, enemy.animation.frame)
      else
        self.sprite:blit(enemy.x, enemy.y)
      end
    end
  end
end

function enemy.startnextwave(self)
  setmetatable(self, self.next):reset()
end

return enemy
