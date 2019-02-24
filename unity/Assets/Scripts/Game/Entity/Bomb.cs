using UnityEngine;
using System.Collections;

public class Bomb : Entity
{
    /// <summary>
    /// Explode after a few seconds
    /// </summary>
    public bool m_singleHitPerEnemy = true;
    public float m_numSecondsBeforeExploding = 3.0f;
    public float m_explosionRadius = 5.0f;
    public float m_throwSpeed = 25.0f;

    public AudioClip m_explodeSfx;

    private readonly Hashtable m_hitEntities = new Hashtable();

    public float ThrowSpeed
    {
        get
        {
            return m_throwSpeed;
        }
    }

    public IEnumerator Explode()
    {
        gameObject.transform.localScale *= m_explosionRadius;

        // Find all entities near the bomb within a certain radius
        foreach (var entity in this.EntitiesWithinRadius(m_explosionRadius))
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

                    Vector2 relativeDir = enemy.transform.position - transform.position;
                    enemy.KnockBack(relativeDir.normalized, 3f, 0.05f);
                }
            }
        }

        EntityRender tickingRenderer = GetTickingRenderer();
        tickingRenderer.enabled = false;

        EntityRender explodeRenderer = GetExplodeRenderer();
        explodeRenderer.SetCurrentGroup("Explode");

        AudioManager.PlaySfx(m_explodeSfx);
        yield return new WaitForSeconds(m_explodeSfx.length);
    }

    private EntityRender GetTickingRenderer()
    {
        return GetComponent<EntityRender>();
    }

    private EntityRender GetExplodeRenderer()
    {
        EntityRender[] entityRenders = GetComponents<EntityRender>();

        if (entityRenders.Length >= 2)
        {
            return entityRenders[1];
        }
        else
        {
            return null;
        }
    }

    protected override void OnStart()
    {
        base.OnStart();

        EntityRender tickingRender = GetTickingRenderer();
        if (tickingRender != null)
        {
            tickingRender.SetFrameInfo("Main", 0, 0, 32, 32);
            tickingRender.SetCurrentGroup("Main");
        }

        EntityRender explosionRender = GetExplodeRenderer();
        if (explosionRender != null)
        {
            for (int i = 0; i < 12; i++)
            {
                explosionRender.SetFrameInfo("Explode", i * 96, 0, 96, 96);
            }

            for (int i = 0; i < 11; i++)
            {
                explosionRender.LinkNextFrame("Explode", i, i + 1);
            }

            explosionRender.SetGlobalFrameDelay(0.15f);
        }

        StartCoroutine(WaitAndExplode());
    }

    private IEnumerator WaitAndExplode()
    {
        yield return new WaitForSeconds(m_numSecondsBeforeExploding);
        yield return Explode();
        RequestDestroy();
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();
    }

}
