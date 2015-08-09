using UnityEngine;
using System;
using System.Collections;

public class GameManagerState_Machine : StateMachine
{
    static public readonly Type InitState = typeof(GameManagerState_Init);
    static public readonly Type PlayingState = typeof(GameManagerState_Playing);
    static public readonly Type DeadState = typeof(GameManagerState_Dead);

    public GameManagerState_Machine(MonoBehaviour behavior)
        : base(behavior)
    {
        RegisterState(InitState);
        RegisterState(PlayingState);
        RegisterState(DeadState);
        SetInitialState(InitState);
    }
}
