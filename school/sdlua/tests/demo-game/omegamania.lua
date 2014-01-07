#!/usr/bin/env lua


local SDL = require"SDL"
local panel = require"panel"
local megasphere = require"megasphere"
local enemy = require"enemies"
local timer = require"timer"
local player = require"player"


local title = {}
local playing = {}
local newship = {}
local lostship = {}
local nextwave = {}


local once = true
function title:draw(dt)
  if once then
    SDL.screen:fill()

    local text
    local w,h
    local x,y

    text = "OMEGAMANIA"
    w,h = SDL.screen.font:measure(text)
    x = 0.5 * (SDL.screen.w - w)
    y = 0.5 * (SDL.screen.h - h) - 50
    SDL.screen:print(x, y, text)

    text = "Powered by SDLua"
    w,h = SDL.screen.font:measure(text)
    x = 0.5 * (SDL.screen.w - w)
    SDL.screen:print(x, y + h, text)

    title.logo:blit(0.5 * (SDL.screen.w - title.logo.w),
      y - title.logo.h - h)

    text = "Press any key to start"
    w,h = SDL.screen.font:measure(text)
    x = 0.5 * (SDL.screen.w - w)
    SDL.screen:print(x, y + 150, text)

    text = "Press <escape> to quit"
    w,h = SDL.screen.font:measure(text)
    x = 0.5 * (SDL.screen.w - w)
    SDL.screen:print(x, y + 150 + h, text)

    SDL.screen:update()

    once = false
  end
end


function title:tick(ticks, dt)
  while ticks > 0 do

    local pressed = SDL.poll"keyboard"
    if pressed.any and not pressed.escape then
      playing.music:volume(0.4):play()
      return newship:tick(ticks, dt)
    end
    if pressed.escape then
      return
    end

    ticks = ticks - 1
  end

  self:draw(dt)

  return self
end


playing.fuel = panel.fuelbar


function playing:draw(dt)
  megasphere:draw(dt)
  enemy:draw(dt)
  player:draw(dt)
  panel:draw(dt)
  SDL.screen:update()
end


playing.consumefuel = timer.gettimer(timer.tick*0.7, playing.fuel.dec, playing.fuel)


playing.alarm = timer.gettimer(10,
  function(f)
    if f.toggle then
      f.fuelColor = {0, 255, 255}
      f.toggle = nil
    else
      f.fuelColor = {0, 128, 128}
      f.toggle = 1
    end
  end, playing.fuel)


function playing:tick(ticks, dt)
  while ticks > 0 do
    megasphere:tick()

    local pressed = SDL.poll"keyboard"
    local axes = SDL.poll"joystick"
    if pressed.escape then
       return
    end
    if pressed.left or (axes.x < -5000) then
      player:move(player.left)
    end
    if pressed.right or (axes.x > 5000) then
      player:move(player.right)
    end
    if pressed.space then
      player:shoot()
    end

    local outoffuel = self.consumefuel()
    if self.fuel.remaining <= 10 then
      self.alarm()
    end

    enemy:tick()
    player:tick()

    if outoffuel then
      self.fuel.fuelColor = {0, 128, 128}
      return lostship:tick(ticks, dt)
    end

    ticks = ticks - 1
  end

  self:draw(dt)

  return self
end


newship.draw = playing.draw


function newship:tick(ticks, dt)
  while ticks > 0 do
    local full = panel.fuelbar:inc()
    
    megasphere:tick()

    local pressed = SDL.poll"keyboard"
    if pressed.escape then
       return
    end

    if full then
      return playing:tick(ticks, dt)
    end

    ticks = ticks - 1
  end

  self:draw(dt)

  return self
end


function lostship:tick(ticks, dt)
  while ticks > 0 do
    local pressed = SDL.poll"keyboard"
    if pressed.escape then
       return
    end

    megasphere:tick()

    panel.ships = panel.ships - 1
    if panel.ships == 0 then
      return
    else
      enemy:reset()
      return newship:tick(ticks, dt)
    end

    ticks = ticks - 1
  end

  self:draw(dt)

  return self
end


nextwave.fuelbar = panel.fuelbar
nextwave.draw = playing.draw


nextwave.consumefuel = timer.gettimer(3, nextwave.fuelbar.dec, nextwave.fuelbar)


function nextwave:tick(ticks, dt)
  while ticks > 0 do
    local outoffuel = self.consumefuel()

    megasphere:tick()

    if outoffuel then
      return newship:ticks(ticks, dt)
    end

    --score = score + enemy.points
    --if score - mark >= 10000 then
      --lives = math.min(lives + 1, 6)
      --mark = score
    --end

    ticks = ticks - 1
  end

  self:draw(dt)

  return self
end


do
  assert(SDL.openscreen(640, 480, "fullscreen", "accelerate")).cursor(false)
  SDL.screen.font = SDL.getfont("data/adore64.ttf", 12)

  title.logo = SDL.getsurface("data/logo.bmp"):keyout()
  playing.music = assert(SDL.getmusic("data/music.mp3"))

  megasphere:init()
  panel:init()
  enemy:init()
  player:init()

  -- throw one away
  timer.fixedtick()

  local state = title
  repeat
    state = state:tick(timer.fixedtick())
    --collectgarbage()
  until state == nil
end

