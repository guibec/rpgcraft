using UnityEngine;
using System;
using System.Collections;

public class EnemyStateMachine : StateMachine
{
    static public readonly Type LiveState = typeof(EnemyState_Live);

    public EnemyStateMachine(MonoBehaviour enemy) : base(enemy)
    {
        RegisterState(LiveState);
        SetInitialState(LiveState);
    }
}
