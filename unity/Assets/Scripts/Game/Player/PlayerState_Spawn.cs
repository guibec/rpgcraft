using UnityEngine;

public class PlayerState_Spawn : State
{
    private Player m_player;

    public PlayerState_Spawn(PlayerStateMachine psm) : base(psm)
    {
    }

    public override void Constructor()
    {
        base.Constructor();
        m_player = FSM.MonoBehaviour as Player;

        m_player.HealthComponent.Revive();
        m_player.transform.position = new Vector2(0, 0);

        SwitchState<PlayerState_Live>();
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