using UnityEngine;
using System.Collections;

public class EntityState_Live : State
{
    private Entity m_entity;

    // knock-back information
    private float m_knockBackTime;
    private Vector2 m_knockBackMovement;

    public EntityState_Live(StateMachine psm)
        : base(psm)
    {
    }

    public override void Constructor()
    {
        base.Constructor();
        m_entity = FSM.MonoBehaviour as Entity;
        m_knockBackTime = 0f;
        m_knockBackMovement = Vector2.zero;
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
        base.Update();
        ApplyKnockBack();
    }

    public void KnockBack(Vector2 movement, float time)
    {
        m_knockBackMovement = movement;
        m_knockBackTime = time;
    }

    private void ApplyKnockBack()
    {
        if (m_knockBackTime > 0.0f)
        {
            if (m_knockBackTime <= Mathf.Epsilon && TimeManager.Dt > 0)
            {
                m_entity.transform.position = (Vector2)m_entity.transform.position + m_knockBackMovement;
                m_knockBackTime = 0f;
                m_knockBackMovement = Vector2.zero;
            }
            else
            {
                float dt = Mathf.Min(m_knockBackTime, TimeManager.Dt);
                Vector2 movement = dt * m_knockBackMovement / m_knockBackTime;

                m_entity.transform.position = (Vector2)m_entity.transform.position + movement;

                // to avoid floating point error
                if (dt >= m_knockBackTime) // should be ==, but let's play safe
                {
                    m_knockBackTime = 0f;
                    m_knockBackMovement = Vector2.zero;
                }
                else
                {
                    m_knockBackTime -= dt;
                    m_knockBackMovement -= movement;
                }
            }
        }
    }

}