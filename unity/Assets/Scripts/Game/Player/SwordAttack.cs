using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class SwordAttack : Entity
{
    /// <summary>
    /// The weapon can only hit an enemy once
    /// </summary>
    public bool m_singleHitPerEnemy = true;

    //private readonly SortedList<Entity, bool> m_hitEntities = new SortedList<Entity, bool>(5);
    private readonly Hashtable m_hitEntities = new Hashtable();

    protected override void OnUpdate()
    {
        base.OnUpdate();
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
                int damage = 1;

                // check if critical hit !
                if (RandomManager.Instance.Probability(0.10f))
                {
                    damage *= 5;
                }

                healthComponent.ReceiveDamage(damage);
                m_hitEntities.Add(other, true);
            }
        }
    }
}
