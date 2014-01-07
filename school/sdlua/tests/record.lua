
local SDLua = require "SDLua"

-- Recorder Data

local status
local normal	= 1
local recording	= 2
local playing	= 3

local i		= 1
local record	= {}
local rx, ry	= 0, 0
local x, y	= 0, 50

local function dup(t)
  local r = {}
  for k,v in pairs(t) do r[k]=v end
  return r
end

local function setstatus(s)
  local text
  status = s

  if s == normal then
    text = "NORMAL"
  end
  if s == recording then
    text = "RECORDING"
  end
  if s == playing then
    text = "PLAYING"
  end

  SDLua.screen:fill({}, {10, 30, SDLua.screen.w, 50}):
    print(10, 30, "STATUS: "..text):update()
end

-- Begin main

local screen = SDLua.openscreen()

screen:print(10, 10,
  "Press [R] to record mouse movement and then press [P] to play it back")

screen.caption("Mouse Recorder")
screen.cursor(false)
screen.grabinput()

setstatus(normal)
repeat
  if status == playing then
    if record[i] ~= nil then
      SDLua.push(record[i])
      i = i + 1
    else
      SDLua.push{pressed="p"}
    end
  end
  
  for event in SDLua.queue do
    if event.pressed then
      if event.pressed == "escape" then
        return
      end
      if event.pressed == "r" then
        if status == recording then
          setstatus(normal)
        else
          rx, ry = x, y
          record = {}
          setstatus(recording)
        end
      elseif event.pressed == "p" then
        if status == playing then
          setstatus(normal)
        elseif #record > 0 then
          setstatus(playing)
          screen:fill({}, {x, y, 100, 20}):update()
          x, y = rx, ry
          i = 1
        end
      end
    end

    if event.moved == "mouse" then
      if status == recording then
        record[#record+1] = dup(event)
      end

      screen:fill({}, {x, y, 100, 20})
      local yrestricted = y + event.y
      if yrestricted < 50 then
        yrestricted = 50
      end

      screen:fill({255, 0, 0}, {x + event.x, yrestricted, 100, 20}):update()
      x, y = x + event.x, yrestricted
    end
  end
until finished
