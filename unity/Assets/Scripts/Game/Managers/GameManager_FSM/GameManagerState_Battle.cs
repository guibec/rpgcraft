using UnityEngine;
using System.Collections;

public class GameManagerState_Battle : State
{
    public GameManagerState_Battle(StateMachine sm) : base(sm) { }

    public override void Constructor()
    {
        base.Constructor();

		AudioManager.Instance.PlayMusic(E_Music.Battle);
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
