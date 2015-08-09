using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
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
    public const int DefaultChunkWidth = 64;
    public const int DefaultChunkHeight = 64;
    public event ChunkChangedEventHandler Changed;

    private TileInfo[,] m_data;

    public Vector2 ChunkPos { get; private set; }

    public ChunkInfo(Vector2 chunkPos)
    {
        ChunkPos = chunkPos;
        m_data = new TileInfo[DefaultChunkHeight, DefaultChunkWidth];
    }

    private TileInfo[,] Data { get { return m_data; } }

    protected void OnChanged(EventArgs e)
    {
        if (Changed != null)
            Changed(this, e);
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

        // Finally, set the area around the player to be grass
        int xCenter = DefaultChunkWidth / 2;
        int yCenter = DefaultChunkHeight / 2;

        // or set them through a circle
        AddCircle(ETile.Grass, xCenter, yCenter, 12);
        sw.Stop();
        Debug.Log(string.Format("ChunkInfo::Generate took {0}ms", sw.ElapsedMilliseconds));
        return true;
    }

    public bool GenerateArena()
    {
        //Stopwatch sw = Stopwatch.StartNew();
        for (int i = 0; i < DefaultChunkWidth; ++i)
        {
            for (int j = 0; j < DefaultChunkHeight; ++j)
            {
                int numTiles = System.Enum.GetNames(typeof(ETile)).Length;

                if (numTiles > 0)
                {
                    //m_slot[i, j] = new Slot(Slot.TerrainType.TT_Forest);
                    int tt = Random.Range(0, numTiles);

                    TileInfo tileInfo = new TileInfo((ETile)tt);
                    WriteSlotValue(i, j, tileInfo);
                }
            }
        }

        //sw.Stop();
        //Debug.Log(string.Format("ChunkInfo::GenerateArena took {0}ms", sw.ElapsedMilliseconds));
        return true;
    }

    private void FillWorldWith(ETile tt)
    {
        //Stopwatch sw = Stopwatch.StartNew();
        for (int i = 0; i < DefaultChunkWidth; ++i)
        {
            for (int j = 0; j < DefaultChunkHeight; ++j)
            {
                WriteSlotValue(i, j, new TileInfo(tt, 0.0f));
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
        List<Vector2> potential = new List<Vector2>(howMany * 2);

        int i = Random.Range(0, DefaultChunkWidth);
        int j = Random.Range(0, DefaultChunkHeight);
        potential.Add(new Vector2(i, j));

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

            Vector2 picked = potential[pick];

            // remove from potential, add to current
            current.Add(picked);
            potential.RemoveAt(pick);

            WriteSlotValue(picked, new TileInfo(tt));

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

            if (found > 0)
            {
                for (int dir = 0; dir < found; ++dir)
                {
                    potential.Add(newPotential[dir]);
                }
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
                        WriteSlotValue(x + i, y + j, new TileInfo(tt));
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
        int howMany = (int)(percent * (float)DefaultChunkWidth * (float)DefaultChunkHeight);

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
        return pos[0] >= 0 && pos[0] < DefaultChunkWidth && pos[1] >= 0 && pos[1] < DefaultChunkHeight;
    }
}



