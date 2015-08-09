using UnityEngine;
using System.Collections;

public class Enemy : Entity
{
    public Enemy()
    {
        
    }

    public HealthComponent HealthComponent { get; private set; }

    protected override void OnAwake()
    {
        base.OnAwake();
        HealthComponent = GetComponent<HealthComponent>();
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
            RequestDestroy();
        }
    }

    //public Interval<int> GoldValue
    //{
    //    get
    //    {
    //        return m_goldValue;
    //    }
    //    set
    //    {
    //        m_goldValue = value;
    //    }
    //}
    //private Interval<int> m_goldValue = new Interval<int>(5, 10);

    //public void Attack(Damage damage_)
    //{
    //    SoundManager.PlaySound("LTTP_Enemy_Hit");

    //    LoseHp(damage_.Amount);
    //    DisplayDamage(damage_.Amount);
    //    KnockBack(damage_);
    //    CheckDeath();
    //}

    //private void DisplayDamage(int amount_)
    //{
    //    int x; int y;
    //    Position.ToPixel(out x, out y);

    //    var text = new DynamicText(String.Format("{0}", -amount_), this, x, y, 0.8f, Color.Red);
    //    text.Velocity = new Unit(0.0f, -1.0f);
    //    text.FontName = "damage";
    //    text.Visible = true;
    //}

    //private void KnockBack(Damage damage_)
    //{
    //    // TODO - If the collision was detected after the object moved through the entity, the knock back will go in the wrong direction
    //    // Entity should keep velocity information which should be automatically computed from the last frame
    //    Unit dir = this.Position - damage_.Source.Position;
    //    dir.Normalize();

    //    // Knock-Back for now, ignore all collision
    //    float knockX = dir.X * 5.0f;
    //    float knockY = dir.Y * 5.0f;

    //    Unit newPos = Position;
    //    newPos.X += knockX;
    //    newPos.Y += knockY;

    //    Position = newPos;
    //}

    //protected override void OnDestroy()
    //{
    //    base.OnDestroy();

    //    SoundManager.PlaySound("LTTP_Enemy_Kill");

    //    int goldGiven = GoldValue.AtRandom();
    //    if (goldGiven > 0)
    //    {
    //        Collectable.GoldCollectable.Spawn(Position, goldGiven);
    //    }
    //}
}
