#!/usr/bin/env lua

local SDL = assert(require"SDL")
local timer = require"timer"
local player = {
  left = -1,
  right = 1
}


function player:init()
  self.sprite = assert(SDL.getsurface"data/ship.bmp"):keyout()
  self.w = self.sprite.w
  self.h = self.sprite.h
  self.x = (SDL.screen.w - self.w) * 0.5
  self.y = SDL.screen.h - 88 - self.h
  self.velocity = 50 / timer.step
  self.dir = 0

  self.shot = assert(SDL.getsound"data/laser.wav")
  self.explode = assert(SDL.getsound"data/boom.wav")

  self.bullet = {}
  local b = self.bullet
  b.sprite = assert(SDL.getsurface"data/shot.bmp"):keyout()
  b.w = b.sprite.w
  b.h = b.sprite.h
  b.velocity = 100 / timer.step
  b.alive = false
end


function player:move(dir)
  self.dir = dir
  if dir == player.left then
    self.x = math.max(0, self.x + player.left * self.velocity)
  elseif dir == player.right then
    self.x = math.min(self.x + player.right * self.velocity,
                      SDL.screen.w - self.w)
  end
end


function player:tick()
  local b = self.bullet
  if b.alive then
    b.y = b.y - b.velocity
    if b.y < 0 then
      b.alive = false
    end
  end
end


function player:draw(dt)
  local b = self.bullet
  if b.alive then
    b.sprite:blit(b.x, b.y + dt * b.velocity)
  end

  self.sprite:blit(self.x, self.y)
end


function player:shoot()
  local b = self.bullet

  if not b.alive then
    b.alive = true
    b.x = self.x + b.w + 3
    b.y = self.y

    self.shot:play(1)
  end
end


return player

