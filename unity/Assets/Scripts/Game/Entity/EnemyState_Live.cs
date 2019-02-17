public class EnemyState_Live : EntityState_Live
{
    private Enemy m_enemy;

    public EnemyState_Live(StateMachine psm)
        : base(psm)
    {
    }

    public override void Constructor()
    {
        base.Constructor();
        m_enemy = FSM.MonoBehaviour as Enemy;
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
        base.Update();
    }

}