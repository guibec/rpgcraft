using UnityEngine;
using System.Collections;

public class GameManagerState_Battle : State
{
    public GameManagerState_Battle(StateMachine sm) : base(sm) { }

    private float m_lastTimeWithEnemyNearby;

    public override void Constructor()
    {
        base.Constructor();

        ChangeBattleMusic();
        m_lastTimeWithEnemyNearby = TimeManager.Now;
    }

    public override void Destructor()
    {
        base.Destructor();
    }

    private void ChangeBattleMusic()
    {
        if (IsBossBattle())
        {
            AudioManager.Instance.PlayMusic(E_Music.BossBattle);
        }
        else
        {
            AudioManager.Instance.PlayMusic(E_Music.Battle);
        }
    }

    private bool IsBossBattle()
    {
        foreach (Enemy e in EntityManager.Instance.Enemies)
        {
            if (e.IsBoss)
            {
                return true;
            }
        }

        return false;
    }

    public override void Update()
    {
        // Condition will become more complex
        // Radius also longer to exit battle mode
        if (CollisionManager.Instance.EnemyWithinPlayerRadius(10))
        {
            m_lastTimeWithEnemyNearby = TimeManager.Now;
            ChangeBattleMusic();
        }
        // To avoid hysteresis of switching between battle and non battle mode
        else if (TimeManager.Now >= m_lastTimeWithEnemyNearby + 5)
        {
            SwitchState<GameManagerState_Playing>();
        }

        base.Update();
    }
}
