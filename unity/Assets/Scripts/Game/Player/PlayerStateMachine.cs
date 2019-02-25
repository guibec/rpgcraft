using UnityEngine;
using System;

public class PlayerStateMachine : StateMachine
{
    public static readonly Type SpawnState = typeof(PlayerState_Spawn);
    public static readonly Type DeadState = typeof(PlayerState_Dead);
    public static readonly Type LiveState = typeof(PlayerState_Live);

    public PlayerStateMachine(MonoBehaviour player) : base(player, new[]{SpawnState, DeadState, LiveState})
    {
    }
}
