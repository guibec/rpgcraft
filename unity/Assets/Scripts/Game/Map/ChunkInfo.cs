using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Random = UnityEngine.Random;

using IronExtension;
using Debug = UnityEngine.Debug;

public struct PatchTemplate
{
    public PatchTemplate(ETile _tile, float _percent, float _tightness)
    {
        tile = _tile;
        percent = _percent;
        tightness = _tightness;
    }
    public ETile tile;
    public float percent;
    public float tightness;
}

public class GenerationTemplate
{
    public List<PatchTemplate> patchTemplate = new List<PatchTemplate>(5);
}

public delegate void ChunkChangedEventHandler(object sender, EventArgs e);

/// <summary>
/// Describe a Chunk of the world
/// </summary>
public class ChunkInfo
{
    public const int m_width = 64;
    public const int m_height = 64;

    static public int Width
    {
        get
        {
            return m_width;
        }
    }

    static public int Height
    {
        get
        {
            return m_height;
        }
    }

    public event ChunkChangedEventHandler Changed;

    private TileInfo[,] m_data;

    // Linked list of potentially occupying entities
    // TODO: See Real Time Collision Detection Chapter 7.1.6 for optimization using a static array instead
    private List<Entity>[,] m_entities;

    enum Direction
    {
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NortWest,
    }

    private Vector2[] m_directionVectors = 
        {
            new Vector2(0, 1),
            new Vector2(1, 1),
            new Vector2(1, 0),
            new Vector2(1, -1),
            new Vector2(0, -1),
            new Vector2(-1, -1),
            new Vector2(-1, 0),
            new Vector2(-1, 1),
    };

    // Each ChunkInfo will keep a list of its neighbors as it makes it easier to navigate between them and
    // have contiguous regions
    private List<ChunkInfo> m_neighbors = new List<ChunkInfo>(Enum.GetNames(typeof(Direction)).Length);

    public ChunkInfo NorthNeighbor
    {
        get
        {
            return m_neighbors[0];
        }
    }

    public ChunkInfo NorthEastNeighbor
    {
        get
        {
            return m_neighbors[1];
        }
    }

    public ChunkInfo EastNeighbor
    {
        get
        {
            return m_neighbors[2];
        }
    }

    public ChunkInfo SouthEastNeighbor
    {
        get
        {
            return m_neighbors[3];
        }
    }

    public ChunkInfo SouthNeighbor
    {
        get
        {
            return m_neighbors[4];
        }
    }

    public ChunkInfo SouthWestNeighbor
    {
        get
        {
            return m_neighbors[5];
        }
    }

    public ChunkInfo WestNeighbor
    {
        get
        {
            return m_neighbors[6];
        }
    }

    public ChunkInfo NorthWestNeighbor
    {
        get
        {
            return m_neighbors[7];
        }
    }

    public Vector2 ChunkPos { get; private set; }

    public GameObject ChunkObject { get; private set; }

    public ChunkInfo(Vector2 chunkPos, GameObject chunkObj)
    {
        ChunkPos = chunkPos;
        ChunkObject = chunkObj;

        m_data = new TileInfo[Height, Width];
        m_entities = new List<Entity>[Height, Width];

        for (int i = 0; i < Enum.GetNames(typeof(Direction)).Length; i++)
        {
            m_neighbors.Add(null);
        }
    }

    private TileInfo[,] Data { get { return m_data; } }

    protected void OnChanged(EventArgs e)
    {
        if (Changed != null)
            Changed(this, e);
    }

    public void PostInitialize()
    {
        for (int i = 0; i < Enum.GetNames(typeof(Direction)).Length; i++)
        {
            Vector2 dir = m_directionVectors[i];
            Vector2 neighborPos = ChunkPos + dir;
            ChunkInfo neighbor = GameManager.Instance.WorldMap.GetChunkFromChunkPos(neighborPos);
            if (neighbor != null)
            {
                m_neighbors[i] = neighbor;
                neighbor.m_neighbors[(i + 4) % 8] = this;
            }
        }
    }

    public void WriteSlotValue(int x, int y, TileInfo tileInfo)
    {
        m_data[y, x] = tileInfo;
        OnChanged(EventArgs.Empty);
    }

    public void WriteSlotValue(Vector2 pos, TileInfo tileInfo)
    {
        WriteSlotValue((int)pos.x, (int)pos.y, tileInfo);
    }

    public TileInfo ReadSlotValue(int x, int y)
    {
        return m_data[y, x];
    }

    public void RemoveEntity(Entity entity, int x, int y)
    {
        List<Entity> list = m_entities[x, y];
        if (list != null)
        {
            list.Remove(entity);
            if (list.Count == 0)
            {
                m_entities[x, y] = null;
            }
        }
    }

    public void AddEntity(Entity entity, int x, int y)
    {
        if (m_entities[x,y] == null)
        {
            m_entities[x, y] = new List<Entity>();
        }

        m_entities[x, y].Add(entity);
    }

    public List<Entity> GetEntities(int x, int y)
    {
        if (m_entities[x,y] == null)
        {
            return null;
        }

        return m_entities[x, y];
    }

    public bool Generate(GenerationTemplate template)
    {
        Stopwatch sw = Stopwatch.StartNew();
        // Completely fill the world with this type of terrain
        FillWorldWith(ETile.Grass);

        // Now, add patches of this type of terrain
        // First argument, how much to fill (ex: 10% of the world)
        // Second argument, how tight are the patch (1.0 = 1 patch, 0 = as many patches as possible).
        // These are just hints

        foreach (var patch in template.patchTemplate)
        {
            AddPatches(patch.tile, patch.percent, patch.tightness);    
        }

        sw.Stop();
        Debug.Log(string.Format("ChunkInfo::Generate took {0}ms", sw.ElapsedMilliseconds));

        string fullDump = "";
        for (int j = 0; j < Width; ++j)
        {
            for (int i = 0; i < Width; ++i)
            {
                fullDump += m_data[i, j].Tile.ToString().Substring(0,1);
            }
            fullDump += "\n";
        }
        Debug.Log(fullDump);

        return true;
    }

    public bool GenerateSquare(ETile tile, int width, int height)
    {
        if (width < 0)
            return false;
        if (height < 0)
            return false;

        Debug.Assert(width <= Width);
        Debug.Assert(height <= Height);

        width = Math.Min(Width, width);
        height = Math.Min(Height, height);

        int startX = Width / 2 - width / 2;
        int startY = Height / 2 - height / 2;

        int endX = startX + width - 1;
        int endY = startY + height - 1;

        for (int j = startY; j <= endY; ++j)
        {
            for (int i = startX; i < endX; ++i)
            {
                WriteSlotValue(i, j, tile);
            }
        }

        return true;
    }

    private void FillWorldWith(ETile tt)
    {
        //Stopwatch sw = Stopwatch.StartNew();
        for (int i = 0; i < Width; ++i)
        {
            for (int j = 0; j < Height; ++j)
            {
                WriteSlotValue(i, j, tt);
            }
        }
        //sw.Stop();
        //Debug.Log(string.Format("ChunkInfo::FillWorldWith took {0}ms", sw.ElapsedMilliseconds));
    }

    private void AddPatches(ETile tt, int howManyPatch, int howManyPerPatch)
    {
        for (int patchCount = 0; patchCount < howManyPatch; patchCount++)
        {
            AddOnePatch(tt, howManyPerPatch);    
        }
    }

    /// <summary>
    /// Add one patch
    /// </summary>
    /// <param name="tt">The type of tile to add</param>
    /// <param name="howMany">How many tiles to add</param>
    private void AddOnePatch(ETile tt, int howMany)
    {
        HashSet<Vector2> current = new HashSet<Vector2>(new Vector2Comparer());
        HashSet<Vector2> potential = new HashSet<Vector2>();

        // Look if there are exiting neighbors first, and start from them if they have the right tiles
        // Look at all directions
        // TODO: This algorithm can be generalized
        if (NorthNeighbor != null)
        {
            for (int x = 0; x < Width; x++)
            {
                if (NorthNeighbor.m_data[x,0].Tile == tt)
                {
                    potential.Add(new Vector2(x, Height-1));
                }
            }
        }
        if (EastNeighbor != null)
        {
            for (int y = 0; y < Height; y++)
            {
                if (EastNeighbor.m_data[0, y].Tile == tt)
                {
                    potential.Add(new Vector2(Width - 1, y));
                }
            }
        }
        if (SouthNeighbor != null)
        {
            for (int x = 0; x < Width; x++)
            {
                if (SouthNeighbor.m_data[x, Height-1].Tile == tt)
                {
                    potential.Add(new Vector2(x, 0));
                }
            }
        }
        if (WestNeighbor != null)
        {
            for (int y = 0; y < Height; y++)
            {
                if (WestNeighbor.m_data[Width-1, y].Tile == tt)
                {
                    potential.Add(new Vector2(0, y));
                }
            }
        }

        // If we had no neighbor with potential, just pick one point at random
        if (potential.Count == 0)
        {
            int i = Random.Range(0, Width);
            int j = Random.Range(0, Height);
            potential.Add(new Vector2(i, j));
        }

        Vector2[] dirs = new Vector2[4];
        dirs[0] = new Vector2(1, 0);
        dirs[1] = new Vector2(-1, 0);
        dirs[2] = new Vector2(0, -1);
        dirs[3] = new Vector2(0, 1);
        Vector2[] newPotential = new Vector2[4];

        // while still some to do and still some place to do it
        while (howMany > 0 && potential.Count > 0)
        {
            howMany--;

            // Pick one guy at random in potential
            int pick = Random.Range(0, potential.Count);
            Vector2 picked = Enumerable.ElementAt(potential, pick);

            // remove from potential, add to current
            current.Add(picked);
            potential.Remove(picked);

            WriteSlotValue(picked, tt);

            // now check all 4 neighbors to see if we can add to new potential
            int found = 0;

            for (int dir = 0; dir < 4; ++dir)
            {
                Vector2 testNeighbor = dirs[dir] + picked;
                if (IsValid(testNeighbor) && !current.Contains(testNeighbor))
                {
                    newPotential[found++] = testNeighbor;
                }
            }

            for (int dir = 0; dir < found; ++dir)
            {
                potential.Add(newPotential[dir]);
            }
        }
    }

    /// <summary>
    /// Add a rectangle of a certain terrain type in the world
    /// </summary>
    /// <param name="tt">Terrain type</param>
    /// <param name="x1">Left coordinate of rectangle</param>
    /// <param name="x2">Right coordinate of rectangle</param>
    /// <param name="y1">Up coordinate of rectangle</param>
    /// <param name="y2">Down coordinate of rectangle</param>
    private void AddRectangle(ETile tt, int x1, int x2, int y1, int y2)
    {
        for (var x = x1; x <= x2; x++)
        {
            for (var y = y1; y <= y2; y++)
            {
                WriteSlotValue(x, y, new TileInfo(tt));
            }
        }
    }

    /// <summary>
    /// Add a circle patch of a certain terrain type in the world
    /// </summary>
    /// <param name="tt">Terrain type</param>
    /// <param name="x">Center of circle X coordinate</param>
    /// <param name="y">Center of circle Y coordinate</param>
    /// <param name="radius">Radius of circle</param>
    private void AddCircle(ETile tt, int x, int y, int radius)
    {
        //Stopwatch sw = Stopwatch.StartNew();

        for (var i = -radius; i <= radius; ++i)
        {
            for (var j = -radius; j <= radius; j++)
            {
                // check if valid region
                Vector2 candidate = new Vector2(x + i, x + j);
                if (IsValid(candidate))
                {
                    // now check if within the circle
                    if (i * i + j * j <= radius*radius)
                    {
                        WriteSlotValue(x + i, y + j, tt);
                    }
                }
            }
        }
        //sw.Stop();
        //Debug.Log(string.Format("ChunkInfo::AddCircle took {0}ms", sw.ElapsedMilliseconds));
    }

    /// <summary>
    /// Add patch of terrain to a world. The parameter are respected in a best-effort way.
    /// </summary>
    /// <param name="tt"></param>
    /// <param name="percent">How much of the world is covered by this terrain</param>
    /// <param name="tightness">How tight that terrain is packed. 0 = as many patches possible, 1 = single patch</param>
    private void AddPatches(ETile tt, float percent, float tightness)
    {
        //Debug.Log(string.Format("Creating {0} patch for {1}% at {2} tightness", tt, percent, tightness));

        Stopwatch sw = Stopwatch.StartNew();
        int howMany = (int)(percent * (float)Width * (float)Height);

        if (howMany == 0)
        {
            sw.Stop();
            Debug.Log(string.Format("ChunkInfo::AddPatches took {0}ms", sw.ElapsedMilliseconds));
            return;
        }
            

        // how much terrain per a given patch
        // some patch may have more than others
        int howManyPerPatch;

        // how many patches
        int howManyPatch;

        if (tightness == 0.0f)
        {
            howManyPerPatch = 1;
            howManyPatch = howMany;
        }
        else
        {
            // 1.0 -> howMany
            // 0.5 -> howMany / 2
            // 0.2 -> howMany / 5
            howManyPerPatch = (int)(howMany * tightness);
            howManyPatch = (int)Mathf.Round((float)howMany / howManyPerPatch);
            if (howManyPatch == 0)
                howManyPatch = 1;
        }

        AddPatches(tt, howManyPatch, howManyPerPatch);
        sw.Stop();
        //Debug.Log(string.Format("ChunkInfo::AddPatches took {0}ms", sw.ElapsedMilliseconds));
    }

    public bool IsValid(Vector2 pos)
    {
        return pos[0] >= 0 && pos[0] < Width && pos[1] >= 0 && pos[1] < Height;
    }
}



