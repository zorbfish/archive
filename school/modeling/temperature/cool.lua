--[[
--  This program models the change in temperature of
--  a cooling cup of coffee using the Euler method.
--]]


-- Returns the elapsed time and the new temperature of the coffee at time t
local function euler(t, coffee, room, r, dt)
  return t + dt, coffee + -r * (coffee - room) * dt
end

-- Output Methods: plot and print
do
  local f

  -- Generates a PNG image of the simulation results
  function plot(t, coffee)
    -- Special Case: generates the plot.png from plot.txt
    if nil == t then
      f:close()
      os.execute("pgnuplot plot.plt")
      return
    end

    -- First run: open the file for storing the coordinate data.
    if 0 == t then
      f = io.open("plot.txt", "w")
    end
    
    -- Write a new coordinate.
    f:write(t, " ", coffee, "\n")
  end

  -- Prints the results of the simulation to stdout
  local _print = print
  function print(t, coffee, room)
    if 0 == t then
      _print("\ntime (min)", "coffee", "coffee - room\n")
    end

    _print(t, coffee, coffee - room)
  end
end

-- Main entry point
do
  local coffee, dt, nshow, r, room, tmax, output

  -- Gather the initial state data for the model
  t, coffee, room = 0, 82.3, 17

  -- Pass either "print" or "plot" method
  io.write "output method = "
  output = _G[io.read("*line")]

  io.write "cooling constant r = "
  r = io.read("*number")

  io.write "time step dt = "
  dt = io.read("*number")

  io.write "duration of run = "
  tmax = io.read("*number")

  io.write "time between output of data = "
  nshow = io.read("*number")
  nshow = math.ceil(nshow / dt)

  -- Prime the results
  output(t, coffee, room)

  -- For each time step output the coffee temperature
  local i = 0
  while t <= tmax do
    t, coffee = euler(t, coffee, room, r, dt)

    i = i + 1
    if 0 == (i % nshow) then
      output(t, coffee, room)
    end
  end

  -- For plot: executes gnuplot using the open file
  if plot == output then plot() end
end
