
local SDLua = require "SDLua"
local event = {}
local before, now

before = SDLua.getticks()
SDLua.openscreen()

repeat
  for event in SDLua.queue do
    if event.quit then return end
  end

  -- Wait 5 seconds before quitting
  now = SDLua.getticks()
  if (now - before) / 1000 >= 5 then
    SDLua.push{quit=true}
  end
until finished