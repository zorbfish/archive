
local lib = require "SDLua"
package.loaded.SDLua = nil
setmetatable(package.loaded, {__mode="v"})
return lib