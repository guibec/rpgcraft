using UnityEngine;
using System.Collections;

public class GameManagerState_Playing : State
{
    public GameManagerState_Playing(StateMachine sm) : base(sm) { }

    public override void Constructor()
    {
        base.Constructor();
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
        if (CollisionManager.Instance.EnemyWithinPlayerRadius(5))
        {
            SwitchState<GameManagerState_Battle>();
        }

        base.Update();
    }
}
