using UnityEngine;
using System.Collections;

public class PlayerState_Live : State
{
    private Player m_player;

    // knock-back information
    private float m_knockBackTime;
    private Vector2 m_knockBackMovement;

    public PlayerState_Live(StateMachine psm)
        : base(psm)
    {
    }

    public override void Constructor()
    {
        base.Constructor();
        m_player = FSM.MonoBehaviour as Player;
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

        m_player.BeforeInputPos = m_player.transform.position;
        m_player.AfterInputPos = m_player.BeforeInputPos;

        if (m_player.HealthComponent.Health <= 0)
        {
            SwitchState<PlayerState_Dead>();
            return;
        }

        Vector3 direction = Vector3.zero;

        if (Input.GetKey(KeyCode.UpArrow) || Input.GetKey(KeyCode.W))
            direction.y = m_player.m_playerSpeed;
        else if (Input.GetKey(KeyCode.DownArrow) || Input.GetKey(KeyCode.S))
            direction.y = -m_player.m_playerSpeed;

        if (Input.GetKey(KeyCode.LeftArrow) || Input.GetKey(KeyCode.A))
            direction.x = -m_player.m_playerSpeed;
        else if (Input.GetKey(KeyCode.RightArrow) || Input.GetKey(KeyCode.D))
            direction.x = m_player.m_playerSpeed;

        direction *= TimeManager.Dt;

        m_player.transform.position = m_player.BeforeInputPos + (Vector2) direction;

        ApplyKnockBack();


        m_player.AfterInputPos = m_player.transform.position;
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
                m_player.transform.position = (Vector2)m_player.transform.position + m_knockBackMovement;
                m_knockBackTime = 0f;
                m_knockBackMovement = Vector2.zero;
            }
            else
            {
                float dt = Mathf.Min(m_knockBackTime, TimeManager.Dt);
                Vector2 movement = dt * m_knockBackMovement / m_knockBackTime;

                m_player.transform.position = (Vector2)m_player.transform.position + movement;

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