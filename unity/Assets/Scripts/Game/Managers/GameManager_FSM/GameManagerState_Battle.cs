using UnityEngine;
using System.Collections;

public class GameManagerState_Battle : State
{
    public GameManagerState_Battle(StateMachine sm) : base(sm) { }

    private float m_lastTimeWithEnemyNearby;

    public override void Constructor()
    {
        base.Constructor();

        AudioManager.Instance.PlayMusic(E_Music.Battle);
        m_lastTimeWithEnemyNearby = TimeManager.Now;
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    public override void Update()
    {
        // Condition will become more complex
        // Radius also longer to exit battle mode
        if (CollisionManager.Instance.EnemyWithinPlayerRadius(10))
        {
            m_lastTimeWithEnemyNearby = TimeManager.Now;
        }
        // To avoid hysteresis of switching between battle and non battle mode
        else if (TimeManager.Now >= m_lastTimeWithEnemyNearby + 5)
        {
            SwitchState<GameManagerState_Playing>();
        }

        base.Update();
    }
}
