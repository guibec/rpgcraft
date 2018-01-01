
-- Sample enemy lua scirpt template thing!
-- (not hooked to anything in engine yet)

local Enemy = ...

function Enemy:Something(dt)
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

function Enemy:Tick(dt)
{
    -- Move me!
    -- Perform some attack?
}

function Enemy:OnSpawn()
{
    -- these invoke C functions:
    Ent_OnTickCo(Overworld.Something)
    Ent_OnTick  (Overworld.Spawner)
}
