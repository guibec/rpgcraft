using UnityEngine;
using System;

public class GameManagerState_Machine : StateMachine
{
    public static readonly Type InitState = typeof(GameManagerState_Init);
    public static readonly Type PlayingState = typeof(GameManagerState_Playing);
    public static readonly Type BattleState = typeof(GameManagerState_Battle);
    public static readonly Type DeadState = typeof(GameManagerState_Dead);

    public GameManagerState_Machine(MonoBehaviour behavior)
        : base(behavior, new[]{ InitState , PlayingState, BattleState, DeadState})
    {
    }
}
