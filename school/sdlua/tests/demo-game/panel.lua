#!/usr/bin/env lua


local SDL = assert(require"SDL")
local panel = {}
local fuelbar = {}
panel.fuelbar = fuelbar


function fuelbar:init()
  self.capacity = 80
  self.remaining = 0
  self.backgroundColor = {128, 0, 0}
  self.fuelColor = {0, 128, 128}

  self.rect = {
      0.5 * (SDL.screen.w - self.capacity * 3),
      SDL.screen.h - 60, self.capacity * 3, 5
  }

  local magenta = {255, 0, 255}
  self.gauge = assert(SDL.getsurface"./data/gauge.bmp"):keyout(magenta)
  self.xgauge = 0.5 * (SDL.screen.w - 248)
  self.ygauge = SDL.screen.h - 64

  self.left = self.xgauge
  self.right = self.xgauge + self.gauge.w
  self.bottom = self.ygauge + self.gauge.h
end


function fuelbar:inc()
  self.remaining = self.remaining + 1
  return self.remaining == self.capacity
end


function fuelbar:dec()
  self.remaining = self.remaining - 1
  return self.remaining == 0
end


function fuelbar:draw()
  SDL.screen:fill(self.backgroundColor, self.rect)
  self.rect[3] = self.remaining * 3
  SDL.screen:fill(self.fuelColor, self.rect)
  self.rect[3] = self.capacity * 3
  self.gauge:blit(self.xgauge, self.ygauge)
end


function panel:init()
  self.fontColor = {64, 64, 64}
  self.backgroundColor = {128, 128, 128}
  self.rect = {0, SDL.screen.h - 80, SDL.screen.w, 80}

  self.ship = assert(SDL.getsurface"./data/reserve.bmp"):keyout()

  self.fuelbar:init()

  self:reset()
end


function panel:reset()
  self.ships = 3
  self.score = 0
end


function panel:draw(dt)
  SDL.screen:fill(self.backgroundColor, self.rect)

  self.fuelbar:draw()

  local x, y = self.fuelbar.left, self.fuelbar.bottom + 4
  local spacing = self.ship.w + 4
  for i = 1,self.ships do
    self.ship:blit(x + (i-1) * spacing, y)
  end

  local text = string.format("%d", self.score)
  local w, h = SDL.screen.font:measure(text)
  SDL.screen.font.bg = self.backgroundColor
  SDL.screen.font.fg = self.fontColor
  SDL.screen:print(self.fuelbar.right - w, y + h, text)
  SDL.screen.font.bg = nil
  SDL.screen.font.fg = nil
end


function panel:tick()
end


return panel

