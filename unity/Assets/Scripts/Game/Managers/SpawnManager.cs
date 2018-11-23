﻿using UnityEngine;
using System.Collections;

public class SpawnManager : MonoSingleton<SpawnManager>
{
    public GameObject m_itemInstancePrefab;
    public GameObject m_slimePrefab;

    private ItemInstance SpawnItem(EItem item)
    {
        if (!m_itemInstancePrefab)
        {
            return null;
        }

        TileResourceDef tileResourceDef = TileMapping.GetTileResourceDef((ETile)item);
        if (tileResourceDef == null)
        {
            return null;
        }

        GameObject obj = Instantiate(m_itemInstancePrefab);

        if (obj == null)
        {
            return null;
        }

        Transform tr = obj.GetComponent<Transform>();
        ItemInstance ii = obj.GetComponent<ItemInstance>();
        MeshRenderer mr = obj.GetComponent<MeshRenderer>();

        if (mr)
        {
            Texture tex = Resources.Load(tileResourceDef.Filename) as Texture;
            if (tex)
            {
                mr.material.mainTexture = tex;
                mr.material.color = ItemInstance.GetColor32ForItem(item);

                float scale = (float) (Screen.height / 2.0) / Camera.main.orthographicSize;
                tr.localScale = new Vector3((float)tex.width / scale, (float)tex.height / scale, tr.localScale.z);
            }

            ii.SetType(item);
        }

        return ii;
    }

    public GameObject SpawnFromTile(ETile tile_, ChunkInfo info_, int x_, int y_)
    {
        EItem toSpawn = EItem.None;
        if (tile_ == ETile.Mountain)
            toSpawn = EItem.Stone;
        else if (tile_ == ETile.Tree)
            toSpawn = EItem.Wood;

        if (toSpawn == EItem.None)
        {
            return null;
        }

        ItemInstance ii = SpawnItem(toSpawn);
        if (!ii)
        {
            return null;
        }

        Vector2 worldPos = GameManager.Chunk2World(info_, x_, y_);
        ii.gameObject.transform.position = new Vector3(worldPos.x, worldPos.y, -0.06f);

        return ii.gameObject;
    }

    public void SpawnLoot(EItem item, Vector2 worldPos)
    {
        ItemInstance ii = SpawnItem(item);
        if (!ii)
        {
            return;
        }

        ii.gameObject.transform.position = new Vector3(worldPos.x, worldPos.y, -0.06f);

        Mover itemInstanceMover = ii.GetComponent<Mover>();
        if (itemInstanceMover)
        {
            // make it moves around in a random direction
            Vector2 randDir = RandomManager.Vector();
            float dir = RandomManager.Next(0.8f, 3.5f);

            Vector2 target = (Vector2)ii.gameObject.transform.position + (randDir * dir);

            itemInstanceMover.StartInterpolation(target, 0.3f, null);
        }
    }

    private GameObject SpawnEnemy()
    {
        if (m_slimePrefab == null)
        {
            return null;
        }

        Vector2 position = GameManager.Instance.MainPlayer.transform.position;

        // also we should spawn them so we don't see them appearing
        // from a x-axis point of view
        // assuming we can see from [ -1 ... 1 ]
        // that means we are interested in spawning them from [ -2 ... -1 , 1 ... 2 ]

        bool xSign = RandomManager.Boolean();
        bool ySign = RandomManager.Boolean();

        float xDistance = RandomManager.Next(0.0f, m_spawnVariableDistance);
        float yDistance = RandomManager.Next(0.0f, m_spawnVariableDistance);

        Vector2 spawnPosition = position +
                                new Vector2(xSign ? m_spawnMinDistance + xDistance : -m_spawnMinDistance - xDistance,
                                    ySign ? m_spawnMinDistance + yDistance : -m_spawnMinDistance - yDistance);

        GameObject obj = Instantiate(m_slimePrefab);

        if (obj == null)
        {
            return null;
        }

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
                m_nextSpawn = RandomManager.Next(SpawnRateMin, SpawnRateMax);
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

    [Tooltip("Variation in distance (0 to m_spawnVariableDistance) in enemy spawning")]
    public float m_spawnVariableDistance = 10.0f;

    [Tooltip("Minimum distance for enemy spawning")]
    public float m_spawnMinDistance = 20.0f;

    float m_spawnTime = 0.0f;
    float m_nextSpawn = 0.0f;
}