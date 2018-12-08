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
        foreach (var entity in CollisionManager.Instance.EntitiesWithinEntityRadius(this, m_explosionRadius))
        {
            // TODO: Shameless copied from SwordAttack - Need to be merged into generic code
            Enemy enemy = entity as Enemy;
            if (enemy && (!m_singleHitPerEnemy || !m_hitEntities.ContainsKey(enemy)))
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
                    m_hitEntities.Add(enemy, true);

                    Vector2 relativeDir = enemy.transform.position - this.transform.position;
                    enemy.KnockBack(relativeDir.normalized, 3f, 0.05f);

                    AudioManager.Instance.PlayHit();
                }
            }
        }
    }

    protected override void OnStart()
    {
        base.OnStart();

        StartCoroutine(WaitAndExplode());
    }

    private IEnumerator WaitAndExplode()
    {
        yield return new WaitForSeconds(m_numSecondsBeforeExploding);
        Explode();
        RequestDestroy();
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();
    }

}
