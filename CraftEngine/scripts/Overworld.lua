
-- quick sample of what we want a lua script to look like:
--  (please modify/fix !)

-- Dunno if this C-provided object definition makes sense.
-- I just borrowed it from 
local Overworld = ...

function Overworld:Something(dt)
{
	while true
		local timeout = 2.0		-- seconds

		print "test 1"
		while timeout > 0 do
			dt = coroutine.yield()
			timeout = timeout - dt
		end

		print "test 2"
		timeout = timeout + 2.0
		while GetTimeInGame() < timeout do
			dt = coroutine.yield()
		end

		print "test 3"
		timeout = timeout + 2.0
		while GetTimeInGame() < timeout do
			dt = coroutine.yield()
		end
	end
}

function Overworld:Tick(dt)
{
	-- age things?

	-- set up some params used by renderer later on.
	-- Could do this using local API passed into lua module
	Overworld:SetTileMapRenderParams(...)
}

function Overworld:Spawner()
{
	-- Do things to spawn world thngs here?
}

function Overworld:Init()
{
	-- these invoke C functions:
	OnTickCo(Overworld.Something)
	OnTick  (Overworld.Spawner)
}
