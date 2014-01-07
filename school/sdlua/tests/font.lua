
local SDLua = require "SDLua"
local event = {}

local f = SDLua.getfont("default")

f.name
f.foreground
f.background
f.size
f.family
f.bold
f.underline
f.italic

w,h = f:dim("This text")