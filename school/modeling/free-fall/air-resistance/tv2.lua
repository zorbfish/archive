-- terminal velocity squared

inp=io.open("tv.txt", "r")
out=io.open("tv2.txt", "a+")

for line in inp:lines() do
  local t, dy = line:match("(.+) (.+)")
  out:write(string.format("%f %f\n", t, dy*dy))
end
inp:close()
out:close()
