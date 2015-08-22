using UnityEngine;
using System.Collections;

public class SpawnManager : MonoSingleton<SpawnManager>
{
    public GameObject m_itemInstancePrefab;
    public GameObject m_slimePrefab;

    private ItemInstance SpawnItem(EItem item)
    {
        if (!m_itemInstancePrefab)
            return null;

        TileResourceDef tileResourceDef = TileMapping.GetTileResourceDef((ETile)item);
        if (tileResourceDef == null)
            return null;

        GameObject obj = Instantiate(m_itemInstancePrefab);

        if (obj == null)
            return null;

        ItemInstance ii = obj.GetComponent<ItemInstance>();
        MeshRenderer mr = obj.GetComponent<MeshRenderer>();

        if (mr)
        {
            Texture tex = Resources.Load(tileResourceDef.Filename) as Texture;
            mr.material.SetTexture(0, tex);

            ii.SetType(item);
        }

        return ii;
    }

    public GameObject SpawnFromTile(ETile tile_, ChunkInfo info_, int x_, int y_)
    {
        ItemInstance ii = SpawnItem(EItem.E_Stone);
        if (!ii)
            return null;

        Vector2 worldPos = GameManager.Chunk2World(info_, x_, y_);
        ii.gameObject.transform.position = new Vector3(worldPos.x, worldPos.y, -0.06f);

        return ii.gameObject;
    }

    public void SpawnLoot(EItem item, Vector2 worldPos)
    {
        ItemInstance ii = SpawnItem(item);
        if (!ii)
            return;

        ii.gameObject.transform.position = new Vector3(worldPos.x, worldPos.y, -0.06f);
    }

    private GameObject SpawnEnemy()
    {
        if (m_slimePrefab == null)
            return null;

        Vector2 position = GameManager.Instance.MainPlayer.transform.position;

        // also we should spawn them so we don't see them appearing
        // from a x-axis point of view
        // assuming we can see from [ -1 ... 1 ]
        // that means we are interested in spawning them from [ -2 ... -1 , 1 ... 2 ]

        // assuming these are the maximum size of the view port
        const float xOffset = 20f;
        const float yOffset = 20f;

        const float distance = 10f;

        bool xSign = RandomManager.Instance.Boolean();
        bool ySign = RandomManager.Instance.Boolean();

        float xDistance = RandomManager.Instance.Next(0.0f, distance);
        float yDistance = RandomManager.Instance.Next(0.0f, distance);

        Vector2 spawnPosition = position +
                                new Vector2(xSign ? xOffset + xDistance : -xOffset - xDistance,
                                    ySign ? yOffset + yDistance : -yOffset - yDistance);

        GameObject obj = Instantiate(m_slimePrefab);

        if (obj == null)
            return null;

        obj.transform.position = spawnPosition;

        return obj;
    }


    protected override void OnInit()
    {
        base.OnInit();

        m_nextSpawn = m_initialSpawnTime;
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();

        //// Look at the state of the world, and decide if we should spawn enemies !
        int enemyCount = EntityManager.Instance.Count<Enemy>();

        if (enemyCount < MaxEnemies)
        {
            if (m_nextSpawn == 0.0f)
            {
                m_nextSpawn = RandomManager.Instance.Next(SpawnRateMin, SpawnRateMax);
            }

            m_spawnTime += TimeManager.Dt;
            if (m_spawnTime >= m_nextSpawn)
            {
                SpawnEnemy();
                m_spawnTime = 0.0f;
                m_nextSpawn = 0.0f;
            }
        }
    }

    [Tooltip("how many enemies of a given type there can be at any time")]
    public int MaxEnemies = 10;

    [Tooltip("how much time (seconds) before new enemies can be spawned")]
    public float SpawnRateMin = 20.0f;

    [Tooltip("how much time (seconds) until new enemies can be spawned")]
    public float SpawnRateMax = 40.0f;

    [Tooltip("how much time (seconds) before the initial spawn ")]
    public float m_initialSpawnTime = 1.0f;

    float m_spawnTime = 0.0f;
    float m_nextSpawn = 0.0f;
}