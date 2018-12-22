using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System;

public class WorldMap : MonoBehaviour
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

    public void Awake()
    {
        m_worldAnchorRoot = GameObject.Find("ChunkRootAnchorPoint").transform;
        m_biomeManager = new BiomeManager();
    }

    public void Start()
    {
        //// For debugging biomes render them
        //MeshRenderer meshRenderer = gameObject.AddComponent<MeshRenderer>();
        //MeshFilter meshFilter = gameObject.AddComponent<MeshFilter>();

        //meshFilter.mesh = m_biomeManager.GetDebugMesh(); ;
    }

    public void Generate()
    {
        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                SpawnChunkAt(i, j);
            }
        }
    }

    public void OnUpdate(Vector2 playerPos)
    {
        m_biomeManager.Update();

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
        GameObject chunkObj = (GameObject)Instantiate(m_worldMapChunkPrefab, spawnPos, spawnRot);
        chunkObj.GetComponent<Renderer>().material.mainTexture = m_tileTextureMap;
        chunkObj.name = string.Format("Chunk({0},{1})", (int)chunkPos.x, (int)chunkPos.y);
        sw.Stop();
        UnityEngine.Debug.Log(string.Format("Mesh Instanciation in {0}ms", sw.ElapsedMilliseconds));

        GenerationTemplate template = new GenerationTemplate();

        // Let's have mountains go from 0.1 to 1.0 depending on the distance from origin

        float distFromOrigin = (float)Math.Sqrt((double)chunkPos.SqrMagnitude());

        float percMountain = distFromOrigin / 10f;
        if (percMountain > 1.0f)
        {
            percMountain = 1.0f;
        }

        template.patchTemplate.Add(new PatchTemplate(ETile.Forest, 0.30f, 0.2f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Desert, 0.20f, 1.0f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Tree, 0.20f, 0.1f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Water, 0.10f, 1.0f));
        template.patchTemplate.Add(new PatchTemplate(ETile.Mountain, percMountain, 0.3f));
        //template.patchTemplate.Add(new PatchTemplate(ETile.Water, 0.20f, 1.0f)); // create Lakes

        ChunkInfo chunkInfo = new ChunkInfo(chunkPos);
        chunkInfo.Generate(template);

        TileMap anotherTileMap = chunkObj.GetComponent<TileMap>();
        anotherTileMap.SourceChunk = chunkInfo;

        // update all mapping
        m_chunks.AddLast(chunkInfo);
        m_posToChunks[new Vector2(chunkPos.x, chunkPos.y)] = chunkInfo;
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

                    // just do one chunk per frame
                    //return;
                }
            }
        }
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
        double newX = (worldPos.x + 32f) / ChunkInfo.DefaultChunkWidth;
        newX = Math.Floor(newX);

        double newY = (worldPos.y + 32f) / ChunkInfo.DefaultChunkHeight;
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
        return Chunk2World(chunk) - new Vector2(ChunkInfo.DefaultChunkWidth / 2, ChunkInfo.DefaultChunkHeight / 2) + new Vector2(x + 0.5f, y + 0.5f);
    }


    public bool GetTileDataFromWorldPos(Vector2 worldPos, out ChunkInfo chunk, out int x, out int y)
    {
        Vector2 chunkIndices = World2Chunk(worldPos);
        Vector2 worldChunkPos = new Vector2(chunkIndices.x * ChunkInfo.DefaultChunkWidth, chunkIndices.y * ChunkInfo.DefaultChunkHeight);

        Vector2 relativePos = new Vector2(worldPos.x, worldPos.y) - worldChunkPos;
        relativePos.x += ChunkInfo.DefaultChunkWidth / 2;
        relativePos.y += ChunkInfo.DefaultChunkHeight / 2;

        ChunkInfo chunkInfo;
        if (m_posToChunks.TryGetValue(chunkIndices, out chunkInfo))
        {
            chunk = chunkInfo;
            x = (int)Math.Floor(relativePos.x);
            y = (int)Math.Floor(relativePos.y);
            return true;
        }

        chunk = null;
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
