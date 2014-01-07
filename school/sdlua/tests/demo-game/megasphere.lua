#!/usr/bin/env lua


local SDL = assert(require"SDL")
local m = {}


function m:init()
  self.rect = {0, 0, SDL.screen.w, SDL.screen.h - 80}
  self.bottom = self.rect[1] + self.rect[4]
  self.backgroundColor = {0, 0, 32}
  self.moving = true
  self.mod = 1

  for i = 1,128 do
    self[i] = {
      x = math.random(0, SDL.screen.w - 1),
      y = math.random(0, self.bottom),
      layer = math.random(1, 4)
    }
    self[i].velocity = 2 / self[i].layer
  end
end


function m:draw(dt)
  if not self.moving then dt = 0 end

  SDL.screen:fill(self.backgroundColor, self.rect)

  for _,star in ipairs(self) do
    SDL.screen:poke(star.x,
                    star.y + dt * self.mod * star.velocity,
                    255 / star.layer,
                    255 / star.layer,
                    255 / star.layer)
  end
end


function m:tick()
  if not self.moving then return end

  for _,star in ipairs(self) do
    star.y = star.y + self.mod * star.velocity
    if star.y >= self.bottom then
      star.y = 0
    end
  end
end


function m:pause()
  self.moving = false
end


function m:resume()
  self.moving = true
end


function m.reposition(o)
  local left = -o.w
  local right = SDL.screen.w + o.w
  if o.x < left then o.x = right end
  if o.x > right then o.x = left end
  if o.y > m.bottom then o.y = m.top end
end


return m

