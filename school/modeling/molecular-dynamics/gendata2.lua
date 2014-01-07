
local n = 64
local lx = 12
local ly = 6

file=io.open("8.2.b.data2", "w")

file:write(string.format("%d %f %f %f %f %f\n", n, 0.01, 10, 1, lx, ly))

for x = 1, 8 do
  local xx = lx/x - 0.5
  for y = 1, 8 do
    local yy = ly/y - 0.5
    file:write(string.format("%f %f %f %f\n", xx, yy, 0, 0))
  end
end

file:close()