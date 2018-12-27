using System.Collections.Generic;
using UnityEngine;
using System.Diagnostics;
using System;
using Debug = UnityEngine.Debug;

public class WorldMap : MonoBehaviourEx
{
    /// <summary>
    /// This contains the list of all spawned chunks
    /// </summary>
    private readonly LinkedList<ChunkInfo> m_chunks = new LinkedList<ChunkInfo>();

    /// <summary>
    /// This is a map between active (x,y) pair and their respective chunks
    /// </summary>
    private readonly Dictionary<Vector2, ChunkInfo> m_posToChunks = new Dictionary<Vector2, ChunkInfo>();

    public Transform m_worldAnchorRoot;
    public GameObject m_worldMapChunkPrefab;
    public Texture m_tileTextureMap;

    /// <summary>
    /// Hold a reference to the biomes for the given world map
    /// </summary>
    private BiomeManager m_biomeManager;

    public Texture BiomeTexture
    {
        get
        {
            return m_biomeManager.DebugTexture;
        }
    }

    public void Awake()
    {
        m_worldAnchorRoot = GameObject.Find("ChunkRootAnchorPoint").transform;
        m_biomeManager = new BiomeManager();
    }

    public void Generate()
    {
        var sw = Stopwatch.StartNew();

        ClearMap();
        m_biomeManager.Generate();

        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                SpawnChunkAt(i, j);
            }
        }

        sw.Stop();
        Debug.Log(string.Format("WorldMap: Initial map generation took {0} ms", sw.ElapsedMilliseconds));
    }

    private void ClearMap()
    {
        m_posToChunks.Clear();
        m_biomeManager.Clear();

        foreach (var chunk in m_chunks)
        {
            Destroy(chunk.ChunkObject);
        }
        m_chunks.Clear();
    }

    public void OnUpdate(Vector2 playerPos)
    {
        // figure out the position of the main character in (x, y) chunk,
        Vector2 chunkPos = World2Chunk(playerPos);
        UpdateChunks(chunkPos);
    }

    private void SpawnChunkAt(int x, int y)
    {
        SpawnChunkAt(new Vector2(x, y));
    }

    private void SpawnChunkAt(Vector2 chunkPos)
    {
        Vector3 spawnPos = m_worldAnchorRoot.transform.position;
        spawnPos.x += chunkPos.x * 64;
        spawnPos.y += chunkPos.y * 64;

        Quaternion spawnRot = m_worldAnchorRoot.transform.rotation;

        Stopwatch sw = Stopwatch.StartNew();

        int x = (int)chunkPos.x;
        int y = (int)chunkPos.y;

        GameObject chunkObj = (GameObject)Instantiate(m_worldMapChunkPrefab, spawnPos, spawnRot);
        chunkObj.GetComponent<Renderer>().material.mainTexture = m_tileTextureMap;
        chunkObj.name = string.Format("Chunk({0},{1})", x, y);
        sw.Stop();
        UnityEngine.Debug.Log(string.Format("Mesh Instanciation in {0}ms", sw.ElapsedMilliseconds));

        ChunkInfo chunkInfo = new ChunkInfo(chunkPos, chunkObj);
        // update all mapping
        m_chunks.AddLast(chunkInfo);
        m_posToChunks[new Vector2(chunkPos.x, chunkPos.y)] = chunkInfo;
        chunkInfo.PostInitialize();

        // Chunk goes from -Inf to + Inf
        // But biomeMap goes from 0 to Width / 2
        // So, this won't work for boundary, but temporary remap by adding half-size
        // TODO: fix me
        int biomeX = x + m_biomeManager.Width / 2;
        int biomeY = y + m_biomeManager.Height / 2;
        EBiome biome = m_biomeManager.Map[biomeX, biomeY];
        GenerationTemplate template = m_biomeManager.GetTemplateFromBiome(biome);
        chunkInfo.Generate(template);

        TileMap anotherTileMap = chunkObj.GetComponent<TileMap>();
        anotherTileMap.SourceChunk = chunkInfo;
    }


    private void UpdateChunks(Vector2 chunkOrigin)
    {
        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                Vector2 newChunkPos = chunkOrigin + new Vector2(i, j);
                if (!IsValidChunk(newChunkPos))
                {
                    SpawnChunkAt(newChunkPos);
                }
            }
        }
    }

    public ChunkInfo GetChunkFromChunkPos(Vector2 chunkPos)
    {
        ChunkInfo chunkInfo;
        if (m_posToChunks.TryGetValue(chunkPos, out chunkInfo))
        {
            return chunkInfo;
        }
        return null;
    }

    public bool IsValidChunk(Vector2 chunkPos)
    {
        return m_posToChunks.ContainsKey(chunkPos);
    }


    /// <summary>
    /// Given a world position, return the corresponding mapped chunk
    /// </summary>
    /// <param name="worldPos"></param>
    /// <returns></returns>
    static public Vector2 World2Chunk(Vector2 worldPos)
    {
        double newX = (worldPos.x + 32f) / ChunkInfo.ChunkWidth;
        newX = Math.Floor(newX);

        double newY = (worldPos.y + 32f) / ChunkInfo.ChunkHeight;
        newY = Math.Floor(newY);

        return new Vector2((float)newX, (float)newY);
    }

    /// <summary>
    /// Given a chunk, return the corresponding worldposition
    /// </summary>
    /// <param name="chunk">The chunk to get the world from</param>
    /// <returns></returns>
    static public Vector2 Chunk2World(ChunkInfo chunk)
    {
        return new Vector2(chunk.ChunkPos.x * 64.0f, chunk.ChunkPos.y * 64.0f);
    }

    static public Vector2 Chunk2World(ChunkInfo chunk, int x, int y)
    {
        return Chunk2World(chunk) - new Vector2(ChunkInfo.ChunkWidth / 2, ChunkInfo.ChunkHeight / 2) + new Vector2(x + 0.5f, y + 0.5f);
    }


    public bool GetTileDataFromWorldPos(Vector2 worldPos, out ChunkInfo chunk, out int x, out int y)
    {
        Vector2 chunkIndices = World2Chunk(worldPos);
        Vector2 worldChunkPos = new Vector2(chunkIndices.x * ChunkInfo.ChunkWidth, chunkIndices.y * ChunkInfo.ChunkHeight);

        Vector2 relativePos = new Vector2(worldPos.x, worldPos.y) - worldChunkPos;
        relativePos.x += ChunkInfo.ChunkWidth / 2;
        relativePos.y += ChunkInfo.ChunkHeight / 2;

        chunk = GetChunkFromChunkPos(chunkIndices);
        if (chunk != null)
        {
            x = (int)Math.Floor(relativePos.x);
            y = (int)Math.Floor(relativePos.y);
            return true;
        }

        x = 0;
        y = 0;
        return false;
    }

    public TileInfo GetTileFromWorldPos(Vector2 worldPos)
    {
        ChunkInfo info;
        int x, y;
        if (GetTileDataFromWorldPos(worldPos, out info, out x, out y))
        {
            return info.ReadSlotValue(x, y);
        }
        else
        {
            return TileInfo.GetInvalid();
        }
    }


}
