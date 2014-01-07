
require 'cool-mod'

	local black = {
	82.3, 78.5, 74.3, 70.7, 67.6, 65.0, 62.5, 60.1,
	58.1, 56.1, 54.3, 52.8, 51.2, 49.9, 48.6, 47.2,
	46.1, 45.0, 43.9, 43.0, 41.9, 41.0, 40.1, 39.5
	}

	local cream = {
	68.8, 64.8, 62.1, 59.9, 57.7, 55.9, 53.9, 52.3,
	50.8, 49.5, 48.1, 46.8, 45.9, 44.8, 43.7, 42.6,
	41.7, 40.8, 39.9, 39.3, 38.6, 37.7, 37.0, 36.4
	}
	
function rms(calc)
		local sum, n = 0, #black
		
		for i = 1, n do
			sum = sum + math.pow(calc[i] - black[i], 2) / n
		end
		
		return math.sqrt(sum)
end
	
local dt = 0.01
local tmax = 46
local nshow = 2

local rmin = 2
local rmax = 3
local rstep = 0.0001

f=io.open("rms.dat", "w")
for r = rmin, rmax, rstep do
	f:write(r / 100, " ", rms(cool(r, dt, tmax, nshow)), "\n")
end
f:close()
