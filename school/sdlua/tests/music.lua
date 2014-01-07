
local SDLua = require "SDLua"

local function waitforend()
  local event = {}
  repeat SDLua.wait(event) until event.musicends
end

SDLua.openscreen()

local music1 = SDLua.getmusic("example.mp3")
local music2 = SDLua.getmusic("example2.mp3")

local function print(ok, msg)
  if not ok then _G.print(msg) end
end

music2:play(1)

print(music1:pause()) --> nil
SDLua.delay(10)
music2:pause()
SDLua.delay(10)
print(music1:resume()) --> nil
music2:resume()

SDLua.delay(10)
print(music2:stop())

waitforend()
print"music2 stopped"
music1:play(1)
waitforend()
print"music1 stopped"