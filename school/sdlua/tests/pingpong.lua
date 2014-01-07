#! /usr/bin/env lua

local SDLua = require "SDL"
local sprite = require "sprite"

local up = -1
local left = -1
local down = 1
local right = 1
local function opposite(dir)
  local op = {[left] = right, [right] = left, [up] = down, [down] = up}
  return op[dir]
end

local screen = SDLua.openscreen()
screen.caption("PingPong")
screen.grabinput()
screen.cursor(false)

local ball = sprite:new(screen.w * 0.5, screen.h * 0.5, 10, 10, {255, 255, 255})
  ball.speed = 3
  ball.xdir = left
  ball.ydir = up
local player = sprite:new((screen.w - 100) * 0.5, screen.h - 50, 100, 10, {255, 0, 0})
local compus = sprite:new((screen.w - 100) * 0.5, 50, 100, 10, {0, 0, 255})

if (math.random(1, 8) % 2) == 0 then
  ball.y = 60
  ball.xdir, ball.ydir = right, down
else
  ball.y = screen.h - 60
end

local volley = 0
local before = SDLua.getticks()
local now = 0
local accum = 0
local step = 1000/60

local bgm = SDLua.getmusic("town.mid"):play()
local bounce = SDLua.getsound("chimes.wav")
local gojingle = SDLua.getmusic("gameover.mp3")

local move = math.random(1, 3)
local function fixedtick()
  ball.x = ball.x + ball.speed * ball.xdir
  ball.y = ball.y + ball.speed * ball.ydir

  if ball.y > (screen.h / 2) then
  else
    if compus.x < ball.x then
      compus.x = compus.x + 10
    end
    if compus.x > ball.x then
      compus.x = compus.x - 10
    end
  end
  if compus.x < 0 then compus.x = 0 end
  if compus.x > (screen.w - compus.w) then compus.x = (screen.w - compus.w) end

  if ball.y < 0 or ball.y > (screen.h - ball.h) then
    local w,h = SDLua.getfont("default"):measure("GAMEOVER")
    screen:print((screen.w - w)/2, (screen.h - h)/2, "GAMEOVER")
    screen:update({(screen.w - w)/2, (screen.h - h)/2, w, h})

    gojingle:play(1)
    local event = {}
    repeat SDLua.wait(event) until event.musicends
    finished = true
    return false
  end

  if ball.x < 0 then
    ball.x = 0
    ball.xdir = opposite(ball.xdir)
  elseif ball.x > (screen.w - ball.w) then
    ball.x = screen.w - ball.w
    ball.xdir = opposite(ball.xdir)
  end

  local comphit = ball:hits(compus)
  if ball:hits(player) or comphit then
    volley = volley + 1
    bounce:play(1)
    local w,h = SDLua.getfont("default"):measure(tostring(volley))
    screen:print((screen.w - w)/2, (20-h)/2, tostring(volley))
    screen:update({0, 0, screen.w, h})
    if volley % 10 == 0 then
      ball.speed = ball.speed + 1
    end

    if comphit then
      move = math.random(1, 3)
    end
  end

  return true
end

repeat
  for event in SDLua.queue do
    if event.pressed == "escape" then
      return
    end
    if event.moved == "mouse" then
      player.x = player.x + event.x
    end
    if player.x < 0 then player.x = 0 end
    if player.x > (screen.w - player.w) then player.x = (screen.w - player.w) end
  end

  local draw
  now = SDLua.getticks()
  total = accum + now - before
  if total >= step then
    while total >= step do
      total = total - step
      draw = fixedtick()
    end
    accum = total
    before = now
  end
  
  if draw then sprite:draw() end
until finished