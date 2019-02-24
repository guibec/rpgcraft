using UnityEngine;
using System;

public class EnemyStateMachine : StateMachine
{
    public static readonly Type LiveState = typeof(EnemyState_Live);

    public EnemyStateMachine(MonoBehaviour enemy) : base(enemy, new[]{LiveState})
    {
    }
}
