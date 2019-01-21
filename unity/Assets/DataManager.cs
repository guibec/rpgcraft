using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DataManager : MonoSingleton<DataManager>
{
    public EnemiesInfo m_enemiesInfo;

    protected override void OnInit()
    {
        m_enemiesInfo = new EnemiesInfo();
    }
}
