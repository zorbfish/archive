
dt = 0.1

y = {}
y["-0.132"]=0
y["0"]=0.075
y["0.1"]=0.260
y["0.2"]=0.535
y["0.3"]=0.870
y["0.4"]=1.27
y["0.5"]=1.73
y["0.6"]=2.23
y["0.7"]=2.77
y["0.8"]=3.35

t = 0.7

for i=1,50 do
  local v = (y[tostring(t+dt)] - y[tostring(t-dt)])/(2*dt)
  local a = (y[tostring(t+dt)] - 2*y[tostring(t)] + y[tostring(t-dt)])/(dt*dt)
  print(t, v, a)

  t = t + dt
  y[tostring(t+dt)]=y[tostring(t)] + v*dt
end