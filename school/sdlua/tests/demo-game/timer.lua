#!/usr/bin/env lua

local SDL = assert(require"SDL")


local timer = {
  tick = 60,
}
timer.step = 1000 / timer.tick


function timer.gettimer(count, f, ...)
  local len = count
  local arg = {...}
  return
    function()
      len = len - 1
      if len == 0 then
        len = count
        return f(unpack(arg))
      end
    end
end


do
  local now
  local last = SDL.getticks()
  function timer.fixedtick()
    now = SDL.getticks()
    local ticks = 0

    while (now - last) > timer.step do
      last = last + timer.step
      ticks = ticks + 1
    end

    return ticks, math.max(0, (now - last) / timer.step)
  end
end


return timer

