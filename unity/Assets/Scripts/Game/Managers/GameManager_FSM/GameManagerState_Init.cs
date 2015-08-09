using UnityEngine;
using System;
using System.Collections;

public class GameManagerState_Init : State
{
    public GameManagerState_Init(GameManagerState_Machine machine) : base(machine)
    {
    }

    public override void Constructor()
    {
        base.Constructor();

        SwitchState<GameManagerState_Playing>();
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
        base.Update();
    }
}
