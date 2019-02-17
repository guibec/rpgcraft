public class DataManager : MonoSingleton<DataManager>
{
    private EnemiesInfo m_enemiesInfo;

    protected override void OnInit()
    {
        m_enemiesInfo = new EnemiesInfo();
    }

    public void Reload()
    {
        OnInit();
    }

    public LootData.LootsInfo LootData => m_enemiesInfo.Data;
}
