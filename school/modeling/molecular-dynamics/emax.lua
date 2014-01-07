
inp=io.open("energy.txt", "r")
out=io.open("emax.txt", "a+")

local e0 -- energy at t = 0
local et -- energy at end interval (in 8.4.a, t = 2)
local t0
local t
local n = 0
et = 0

-- determine the timestep used for this data file
local s = inp:read("*line")
t0, e0 = s:match("(.+) (.+)")
s = inp:read("*line")
t = s:match("(.+) .+")

-- find the last line of the data file
local last
for line in inp:lines() do
  et = et + tonumber(line:match(". (.+)"))
  n = n + 1
end
inp:close()

-- grab the energy value
et = et / n


-- store a new entry in the plot file in the form [timestep, energy]
out:write(string.format("%f %f\n", t - t0, math.abs(et - e0)))

out:close()