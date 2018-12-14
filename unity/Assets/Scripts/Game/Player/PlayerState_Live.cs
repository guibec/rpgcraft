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
            GameManager.Instance.OnMainPlayerDead();
            return;
        }

        // Compute actual Player speed
        float speed = m_player.PlayerSpeed;

        TileInfo tileInfo = GameManager.Instance.GetTileFromWorldPos(m_player.transform.position);
        TileProperties tileProperties = TileMapping.GetTileProperties(tileInfo.Tile);

        if (tileProperties != null)
        {
            speed *= tileProperties.SpeedFactor;
        }

        Vector3 direction = Vector3.zero;
        if (Input.GetKey(KeyCode.UpArrow) || Input.GetKey(KeyCode.W))
            direction.y = speed;
        else if (Input.GetKey(KeyCode.DownArrow) || Input.GetKey(KeyCode.S))
            direction.y = -speed;

        if (Input.GetKey(KeyCode.LeftArrow) || Input.GetKey(KeyCode.A))
            direction.x = -speed;
        else if (Input.GetKey(KeyCode.RightArrow) || Input.GetKey(KeyCode.D))
            direction.x = speed;

        direction *= TimeManager.Dt;

        m_player.transform.position = m_player.BeforeInputPos + (Vector2) direction;

        HandleAutoAttack();

        base.Update();

        m_player.AfterInputPos = m_player.transform.position;
    }

    private void HandleAutoAttack()
    {
        // Enable auto-attack
        if (EntityManager.Instance.Count<SwordAttack>() == 0)
        {
            Enemy closest = null;
            float closestSqrDistance = float.MaxValue;
            foreach (Entity e in EntityManager.Instance.Entities)
            {
                Enemy candidate = e as Enemy;
                if (candidate == null)
                    continue;

                float sqrDistance = (candidate.transform.position - m_player.transform.position).sqrMagnitude;
                if (sqrDistance < closestSqrDistance)
                {
                    closest = candidate;
                    closestSqrDistance = sqrDistance;
                }
            }

            if (closest != null && closestSqrDistance <= 4.0f * 4.0f)
            {
                m_player.SpawnAttackToward(m_player.transform.position, closest.transform.position);
            }
        }
    }
}