
local t = setmetatable({}, {__mode="v"})
for k,v in pairs(package.loaded) do
  t[k] = v
end
package.loaded = t

local SDLua = require "SDLua"
local event = {}

local screen = SDLua.openscreen(148, 148)
local logo = SDLua.getsurface("logo.gif")

screen:fill()
local clip = screen:clip({30, 30, 88, 88})

print(string.format("Old screen clipspace (%d, %d) to (%d, %d)",
  clip[1], clip[2], clip[3], clip[4]))

logo:blit(10, 10).update(screen)

clip = screen:clip()

print(string.format("Old screen clipspace (%d, %d) to (%d, %d)",
  clip[1], clip[2], clip[3], clip[4]))

repeat
  SDLua.wait(event)
until event.quit