using UnityEngine;
using System.Collections;

public class SpawnManager : MonoSingleton<SpawnManager>
{
    public GameObject m_itemInstancePrefab;
    public GameObject m_slimePrefab;

    public GameObject SpawnFromTile(ETile tile_, ChunkInfo info_, int x_, int y_)
    {
        if (m_itemInstancePrefab == null)
            return null;

        GameObject obj = Instantiate(m_itemInstancePrefab);

        if (obj == null)
            return null;

        ItemInstance ii = obj.GetComponent<ItemInstance>();
        MeshRenderer mr = obj.GetComponent<MeshRenderer>();

        if (mr)
        {
            Texture tex = Resources.Load("Item_3") as Texture;
            mr.material.SetTexture(0, tex);

            ii.SetType(EItem.E_Stone);
        }


        Vector2 worldPos = GameManager.Chunk2World(info_, x_, y_);
        obj.transform.position = new Vector3(worldPos.x, worldPos.y, -0.06f);

        return obj;
    }

    private GameObject SpawnEnemy()
    {
        if (m_slimePrefab == null)
            return null;

        //Unit pos = Unit.AtRandom(new Unit(0, 0), new Unit(WorldManager.Instance.CurrentWorld.Width, WorldManager.Instance.CurrentWorld.Height));

        Vector2 position = GameManager.Instance.MainPlayer.transform.position;

        const float distance = 10.0f;

        float xOffset = RandomManager.Instance.Next(0.0f, distance) - distance / 2.0f;
        float yOffset = RandomManager.Instance.Next(0.0f, distance) - distance / 2.0f;

        Vector2 spawnPosition = position + new Vector2(xOffset, yOffset);

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