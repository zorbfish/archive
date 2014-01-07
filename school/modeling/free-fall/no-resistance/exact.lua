
local g = 9.8
local y = 20
local v = 0
local t = 0

while y >= 0 do
  v = v - g*t
  y = y + v*t - 0.5*g*t*t
  print(t, y, v)
  t = t + 0.1
end
print(t, y, v)