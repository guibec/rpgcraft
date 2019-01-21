using UnityEngine;
using System.Collections;

public class Enemy : Entity
{
    public Enemy()
    {
        m_fsm = new EnemyStateMachine(this);
    }

    public HealthComponent HealthComponent { get; private set; }
    public bool IsBoss = false;

    protected override void OnAwake()
    {
        base.OnAwake();
        HealthComponent = GetComponent<HealthComponent>();
    }

    [SerializeField]
    private string m_name;

    public string Name
    {
        get
        {
            if (!string.IsNullOrEmpty(m_name))
            {
                return m_name;
            }
            else
            {
                return this.GetType().Name;
            }
        }
    }

    public override void OnTouch(Entity other)
    {
        base.OnTouch(other);

        Player asPlayer = other as Player;
        if (asPlayer != null)
        {
            if (asPlayer.CanReceiveDamage())
            {
                asPlayer.ReceiveDamage(5);

                // check relative position of player vs enemy
                Vector2 relativeDir = other.transform.position - this.transform.position;
                asPlayer.KnockBack(relativeDir.normalized, 3f, 0.05f);
            }
        }
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();

        if (HealthComponent.Health <= 0)
        {
            if (!IsDestroying())
            {
                // give XP to main player
                RewardWithXP();

                // OnKilled mean the enemy was killed as part of the gameplay, this is different from calling
                // RequestDestroy which just means we want the enemy go to go away and disappear
                OnKilled();
            }
        }
    }

    private void OnKilled()
    {
        EnemyInfo enemyInfo = DataManager.Instance.m_enemiesInfo.GetInfoFromName(Name);

        if (enemyInfo != null)
        {
            foreach (EItem item in enemyInfo.RandomLoot())
            {
                SpawnManager.Instance.SpawnLoot(item, transform.position);
            }
        }

        RequestDestroy();
    }

    private void RewardWithXP()
    {
        int xp = 10;
        GameManager.Instance.MainPlayer.Experience.AddXP(xp);

        GameObject anchor = GameManager.Instance.MainPlayer.gameObject;
        Vector2 screenPos = Camera.main.WorldToScreenPoint(anchor.transform.position);
        screenPos.y += 32;

        UIManager.DynamicText dynamicText = UIManager.Instance.DisplayTextWithDuration(string.Format("+{0} XP", xp), screenPos, 2f);
        dynamicText.Text.color = Color.green;

        dynamicText.ParentToWorld(anchor);
        dynamicText.SimpleMovement(new Vector2(0f, 16f), 1f);
    }

    protected override void OnEntityDestroy()
    {


        base.OnEntityDestroy();
    }
}
