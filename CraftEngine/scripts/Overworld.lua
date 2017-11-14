
-- quick sample of what we want a lua script to look like:
--  (please modify/fix !)

local Overworld = ...

function Overworld:Something()
{
	print "test 1"
	coroutine.yield()
	print "test 2"
	coroutine.yield()
	print "test 3"
}

function Overworld:Init(float dt)
{
	Entity_OnTick(Overworld.Tick);
}

function Overworld:Tick(float dt)
{
	-- age things?
	
	if some_condition then
		Entity_AddCoTick(Overworld.Something);
	end
	
	-- set up some params used by renerer later on.
	-- Could do this using local API passed into lua module
	Overworld:SetTileMapRenderParams(...)
}

