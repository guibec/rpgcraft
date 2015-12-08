using UnityEngine;
using System.Collections;

public class PlayerState_Live : EntityState_Live
{
    private Player m_player;

    public PlayerState_Live(StateMachine psm)
        : base(psm)
    {
    }

    public override void Constructor()
    {
        base.Constructor();
        m_player = FSM.MonoBehaviour as Player;
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
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

        base.Update();

        m_player.AfterInputPos = m_player.transform.position;
    }
}