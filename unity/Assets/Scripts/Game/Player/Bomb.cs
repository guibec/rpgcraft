using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Bomb : Entity
{
    /// <summary>
    /// Explode after a few seconds
    /// </summary>
    public bool m_singleHitPerEnemy = true;
    public float m_numSecondsBeforeExploding = 3.0f;
    public float m_explosionRadius = 10.0f;

    private readonly Hashtable m_hitEntities = new Hashtable();

    public void Explode()
    {
        // Find all entities near the bomb within a certain radius
        //foreach (var entity in this.EntitiesWithinRadius(m_explosionRadius))
        //{

        //}
    }

    public override void OnTouch(Entity other)
    {
        base.OnTouch(other);

        // to prevent another attack while being destroyed
        // this might need to be moved into the OnTouch collision system ...
        if (IsDestroying())
            return;

        Enemy enemy = other as Enemy;
        if (enemy && (!m_singleHitPerEnemy || !m_hitEntities.ContainsKey(other)))
        {
            HealthComponent healthComponent = enemy.HealthComponent;
            if (healthComponent.CanReceiveDamage())
            {
                // simple damage formula for now
                int damage = GameManager.Instance.MainPlayer.Experience.Level;

                // check if critical hit !
                if (RandomManager.Probability(0.10f))
                {
                    damage *= 3;
                }

                healthComponent.ReceiveDamage(damage);
                m_hitEntities.Add(other, true);

                Vector2 relativeDir = other.transform.position - this.transform.position;
                enemy.KnockBack(relativeDir.normalized, 3f, 0.05f);

                AudioManager.Instance.PlayHit();
            }
        }
    }
}
