using UnityEngine;
using System;

public class PlayerStateMachine : StateMachine
{
    static public readonly Type SpawnState = typeof(PlayerState_Spawn);
    static public readonly Type DeadState = typeof(PlayerState_Dead);
    static public readonly Type LiveState = typeof(PlayerState_Live);

    public PlayerStateMachine(MonoBehaviour player) : base(player)
    {
        RegisterState(SpawnState);
        RegisterState(DeadState);
        RegisterState(LiveState);
        SetInitialState(SpawnState);
    }
}
