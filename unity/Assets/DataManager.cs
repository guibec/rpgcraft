using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DataManager : MonoSingleton<DataManager>
{
    private EnemiesInfo m_enemiesInfo;

    protected override void OnInit()
    {
        m_enemiesInfo = new EnemiesInfo();
    }

    public LootData.LootsInfo LootData
    {
        get
        {
            return m_enemiesInfo.LootData;
        }
    }
}
