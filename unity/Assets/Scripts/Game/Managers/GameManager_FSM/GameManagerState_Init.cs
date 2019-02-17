public class GameManagerState_Init : State
{
    public GameManagerState_Init(GameManagerState_Machine machine) : base(machine)
    {
    }

    public override void Constructor()
    {
        base.Constructor();

        UIManager.Instance.HideGameOverMessage();

        // remove all entities on the screen
        EntityManager.Instance.RemoveAll<Enemy>();
        EntityManager.Instance.RemoveAll<ItemInstance>();

        SwitchState<GameManagerState_Playing>();
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
