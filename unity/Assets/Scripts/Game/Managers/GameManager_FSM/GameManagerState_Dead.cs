using UnityEngine;
using System.Collections;

public class GameManagerState_Dead : State
{
    public GameManagerState_Dead(StateMachine sm) : base(sm) { }

    public override void Constructor()
    {
        base.Constructor();
        UIManager.Instance.DisplayGameOverMessage(new Vector2(Screen.width * 0.5f, Screen.height * 0.5f), "You are dead!");
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
