using System;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;
using Debug = UnityEngine.Debug;

public class BiomeMap
{
    private const int m_width = 1024;
    private const int m_height = 1024;
    private readonly EBiome[,] m_biomes = new EBiome[m_width, m_height];

    public int Width => m_width;

    public int Height => m_height;

    public EBiome this[int x, int y]
    {
        get
        {
            Debug.Assert(x >= 0 && x < Width);
            Debug.Assert(y >= 0 && y < Height);
            return m_biomes[x, y];
        }
        set => m_biomes[x, y] = value;
    }

    public void Clear()
    {
        Array.Clear(m_biomes, 0, m_biomes.Length);
    }
}

public class BiomeManager
{
    private const int NumPoints = 1024;

    List<Vector2> m_points;
    Texture m_debugTexture;
    readonly BiomeMap m_biomeMap = new BiomeMap();

    /// <summary>
    /// How each Biome map to a generation template tells us how this area will be generated
    /// </summary>
    private readonly Dictionary<EBiome, GenerationTemplate> m_biomeToGeneration = new Dictionary<EBiome, GenerationTemplate>(10);

    public BiomeMap Map => m_biomeMap;

    public int Width => Map.Width;

    public int Height => Map.Height;

    public Texture DebugTexture
    {
        get
        {
            if (m_debugTexture == null)
            {
                GenerateDebugTexture();
            }

            return m_debugTexture;
        }
    }

    public BiomeManager()
    {
        InitializeGenerationTemplates();
    }

    private void InitializeGenerationTemplates()
    {
        // TODO: Could be data driven
        GenerationTemplate plainTemplate = new GenerationTemplate();
        //plainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Forest, 0.02f, 0.0f));
        //plainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Water, 0.02f, 0.0f));
        m_biomeToGeneration[EBiome.Plain] = plainTemplate;

        GenerationTemplate oceanTemplate = new GenerationTemplate();
        oceanTemplate.patchTemplate.Add(new PatchTemplate(ETile.Water, 1.0f, 1.0f));
        m_biomeToGeneration[EBiome.Ocean] = oceanTemplate;

        GenerationTemplate forestTemplate = new GenerationTemplate();
        forestTemplate.patchTemplate.Add(new PatchTemplate(ETile.Forest, 1.0f, 1.0f));
        m_biomeToGeneration[EBiome.Forest] = forestTemplate;

        GenerationTemplate desertTemplate = new GenerationTemplate();
        desertTemplate.patchTemplate.Add(new PatchTemplate(ETile.Desert, 1.0f, 1.0f));
        m_biomeToGeneration[EBiome.Desert] = desertTemplate;

        GenerationTemplate mountainTemplate = new GenerationTemplate();
        mountainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Mountain, 1.0f, 1.0f));
        m_biomeToGeneration[EBiome.Mountain] = mountainTemplate;
    }

    static public ETile Biome2Tile(EBiome biome)
    {
        switch (biome)
        {
            case EBiome.Plain:
                return ETile.Grass;
            case EBiome.Ocean:
                return ETile.Water;
            case EBiome.Forest:
                return ETile.Tree;
            case EBiome.Desert:
                return ETile.Desert;
            case EBiome.Mountain:
                return ETile.Mountain;
            default:
                return ETile.Grass;
        }
    }

    public GenerationTemplate GetTemplateFromBiome(EBiome biome)
    {
        return m_biomeToGeneration[biome];
    }

    public void Generate()
    {
        m_points = new List<Vector2>(Width * Height);

        for (int i = 0; i < NumPoints; i++)
        {
            Vector2 point = new Vector2(RandomManager.Next(0, 1.0f), RandomManager.Next(0, 1.0f));
            m_points.Add(point);
        }

        voronoiTesselate();
    }

    public void Clear()
    {
        Map.Clear();
        m_points?.Clear();
        m_debugTexture = null;
    }

    // Implement Voronoi tessellation 
    // https://en.wikipedia.org/wiki/Voronoi_diagram#/media/File:Voronoi_growth_euclidean.gif
    private class RegionDistance
    {
        public RegionDistance()
        {
            Region = -1;
            Distance = 0;
        }

        public RegionDistance(int region, int distance)
        {
            Region = region;
            Distance = distance;
        }

        public int Region
        {
            get;
            set;
        }

        public int Distance
        {
            get;
            set;
        }
    }

    private void voronoiTesselate()
    {
        Stopwatch sw = Stopwatch.StartNew();

        RegionDistance[,] regionDistances = new RegionDistance[Width, Height];

        // Remap each of the point into the biome. It's possible some will overwrite each other, that's fine, shouldn't happen too much.
        var closePoints = new Dictionary<Vector2, RegionDistance>();
        int counter = 1;
        foreach (Vector2 point in m_points)
        {
            int x = (int)(point.x * Width);
            int y = (int)(point.y * Height);

            // Should be fine since point coordinates are supposed to be [0.0, 1.0[
            Debug.Assert(x >= 0 && x < Width);
            Debug.Assert(y >= 0 && y < Height);

            regionDistances[x, y] = new RegionDistance(counter++, 0);
            closePoints[new Vector2(x, y)] = regionDistances[x,y];
        }

        // time to fill the missing regions !
        // I can flood fill to figure out all the regions
        // This is a O(Width*Height) complexity algorithm. It will also use up to O(Width*Height) memory for optimization
        Stopwatch minDistance = Stopwatch.StartNew();

        Vector2[] dirs =
        {
            new Vector2(-1, -1),
            new Vector2(-1, 0),
            new Vector2(-1, 1),
            new Vector2(-1, 0),
            new Vector2(1, 0),
            new Vector2(1, -1),
            new Vector2(1, 0),
            new Vector2(1,1),
        };

        // regionDistances keep track of the filled area
        // closePoints keep track of which points to grow from

        while (closePoints.Count > 0)
        {
            var newPoints = new Dictionary<Vector2, RegionDistance>(128);

            foreach (var point in closePoints)
            {
                // look at the 8 neighbors and update closePoints
                foreach (var dir in dirs)
                {
                    Vector2 neighborVector = point.Key + dir;
                    if (neighborVector.x < 0 || neighborVector.y < 0 || neighborVector.x >= Width || neighborVector.y >= Height)
                        continue;

                    if (regionDistances[(int)neighborVector.x, (int)neighborVector.y] != null)
                        continue;

                    RegionDistance rdCurrent = point.Value;
                    regionDistances[(int)neighborVector.x, (int)neighborVector.y] = new RegionDistance(rdCurrent.Region, rdCurrent.Distance + 1);
                    newPoints[new Vector2(neighborVector.x, neighborVector.y)] = regionDistances[(int)neighborVector.x, (int)neighborVector.y];
                }
            }

            // restart with the new newPoints
            closePoints = newPoints;
        }
        minDistance.Stop();

        // For now just map all region back to EBiome
        Stopwatch remap = Stopwatch.StartNew();
        int enumLength = Enum.GetNames(typeof(EBiome)).Length; // keeping this outside the loop actually save 1000ms for 1M elements
        for (int j = 0; j < Height; j++)
        {
            for (int i = 0; i < Width; i++)
            {
                int value = regionDistances[i,j].Region;
                value %= enumLength;
                Map[i, j] = (EBiome)value;
            }
        }
        remap.Stop();

        GenerateDebugTexture();
        sw.Stop();
        Debug.Log($"BiomeManager: Voronoi tessellation took {sw.ElapsedMilliseconds} ms");
        Debug.Log($"BiomeManager: Voronoi tessellation min distance took {minDistance.ElapsedMilliseconds} ms");
        Debug.Log($"BiomeManager: Voronoi tessellation remap took {remap.ElapsedMilliseconds} ms");
    }

    private void GenerateDebugTexture()
    {
        // Create a new 2x2 texture ARGB32 (32 bit with alpha) and no mipmaps
        var texture = new Texture2D(Width, Height, TextureFormat.ARGB32, false);

        for (int j = 0; j < Height; j++)
        {
            for (int i = 0; i < Width; i++)
            {
                Color pixelColor;

                switch (Map[i,j])
                {
                    case EBiome.Plain:
                        pixelColor = Color.green;
                        break;
                    case EBiome.Ocean:
                        pixelColor = Color.blue;
                        break;
                    case EBiome.Forest:
                        pixelColor = Color.grey;
                        break;
                    case EBiome.Desert:
                        pixelColor = Color.red;
                        break;
                    case EBiome.Mountain:
                        pixelColor = Color.black;
                        break;
                    //case EBiome.Snow:
                    //    pixelColor = Color.white;
                    //    break;
                    //case EBiome.Jungle:
                    //    pixelColor = Color.magenta;
                    //    break;
                    default:
                        pixelColor = Color.cyan;
                        break;
                }

                texture.SetPixel(i, j, pixelColor);
            }
        }

        // Apply all SetPixel calls
        texture.Apply();

        m_debugTexture = texture;
    }

 }
