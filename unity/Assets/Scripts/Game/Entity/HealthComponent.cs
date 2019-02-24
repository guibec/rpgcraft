using System;
using UnityEngine;

public class HealthComponent : MonoBehaviourEx 
{
    public delegate void HealthChangedEventHandler(object sender, EventArgs e);
    public event HealthChangedEventHandler HealthChanged;

    public int m_defaultHealth = 30;

    public int DefaultHealth
    {
        get { return m_defaultHealth; }
        private set { m_defaultHealth = value; }
    }

    /// <summary>
    /// how much time you are invincible between attack
    /// </summary>
    [/*Inspect,*/ SerializeField]
    private readonly float m_interdamageTime = 2.0f;

    /// <summary>
    /// prevent multiple hits when touching enemies
    /// </summary>
    private float m_damageTimer;

    //[Inspect]
    private int m_health;
    public int Health
    {
        get
        {
            return m_health;
        }
        set
        {
            int previousHealth = m_health;
            m_health = Math.Max(value, 0);

            if (m_health != previousHealth)
            {
                OnHealthChanged();
            }
        }
    }

    public void Start()
    {
        Health = DefaultHealth;
    }

    private void OnHealthChanged()
    {
        HealthChanged?.Invoke(this, EventArgs.Empty);
    }

    public void ReceiveDamage(int damage)
    {
        Health -= damage;
        m_damageTimer = m_interdamageTime;

        Vector2 screenPos = Camera.main.WorldToScreenPoint(transform.position);
        screenPos.y += 32;

        UIManager.DynamicText dynamicText = UIManager.Instance.DisplayTextWithDuration((-damage).ToString(), screenPos, 2f);
        dynamicText.ParentToWorld(gameObject);
        dynamicText.SimpleMovement(new Vector2(0f, 16f), 1f);
    }

    public void ReceiveHeal(int heal)
    {
        Health += heal;

        Vector2 screenPos = Camera.main.WorldToScreenPoint(transform.position);
        screenPos.y += 32;

        UIManager.DynamicText dynamicText = UIManager.Instance.DisplayTextWithDuration("+" + heal, screenPos, 2f);
        dynamicText.ParentToWorld(gameObject);
        dynamicText.SimpleMovement(new Vector2(0f, 16f), 1f);
    }

    public bool CanReceiveDamage()
    {
        return Health > 0 && m_damageTimer <= 0f;
    }

    private void HandleDamageTimer()
    {
        m_damageTimer = Mathf.Max(m_damageTimer - TimeManager.Dt, 0f);
    }

    public void Revive()
    {
        Health = DefaultHealth;
    }

    private void Update()
    {
        HandleDamageTimer();
    }

}
