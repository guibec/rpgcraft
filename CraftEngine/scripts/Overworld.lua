
-- quick sample of what we want a lua script to look like:
--  (please modify/fix !)

local Overworld = ...

function Overworld:DoSomething(dt)
{
    while true
        local timeout = 2.0     -- seconds

        print "test 1"
        while timeout > 0 do
            dt = coroutine.yield()
            timeout = timeout - dt
        end

        print "test 2"
        timeout = timeout + 2.0
        while timeout > 0 do
            dt = coroutine.yield()
            timeout = timeout - dt
        end

        print "test 3"
        timeout = timeout + 2.0
        while timeout > 0 do
            dt = coroutine.yield()
            timeout = timeout - dt
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

function Overworld:OnSpawn()
{
    -- these invoke C functions:
    Ent_OnTickCo(Overworld.DoSomething)
    Ent_OnTick  (Overworld.Tick)
}
