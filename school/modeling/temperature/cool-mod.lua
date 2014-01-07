--[[
	This program models the change in temperature
	of cooling cup of coffee using the euler method.
--]]


-- Returns the elapsed time and the new temperature of the coffee at time t
local function euler(t, coffee, room, r, dt)
	return t + dt, coffee + -r * dt * (coffee - room)
end

function cool(r, dt, tmax, nshow)
	local coffee, room, calc
	calc = {}
	
	-- Gather the initial state data for the model
	t, coffee, room = 0, 90, 17
	
	-- For each time step output the coffee temperature
	local i = 0
	while t < tmax do
		t, coffee = euler(t, coffee, room, r, dt)
		i = i + 1
		
		-- Store the calculated temperature
		if 0 == (i % nshow) then
			calc[#calc+1] = coffee
		end
	end
	
	return calc
end
