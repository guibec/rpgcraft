using UnityEngine;
using System.Collections;

public class PlayerState_Dead : State
{
    public PlayerState_Dead(StateMachine sm) : base(sm) { }

    private float m_playerDeathTime;

    public override void Constructor()
    {
        base.Constructor();

        m_playerDeathTime = Mathf.Max(GameManager.Instance.PlayerDeathTime, 0.01f);
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
        base.Update();

        if (m_playerDeathTime > 0f)
        {
            m_playerDeathTime -= TimeManager.Dt;
        }

        if (m_playerDeathTime <= 0f)
        {
            SwitchState<PlayerState_Spawn>();
            GameManager.Instance.OnMainPlayerSpawn();
        }
    }
}
