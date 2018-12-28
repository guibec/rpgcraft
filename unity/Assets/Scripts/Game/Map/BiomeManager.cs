using System;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;

using IronExtension;
using Debug = UnityEngine.Debug;

public class BiomeMap
{
    private const int m_width = 1024;
    private const int m_height = 1024;
    private readonly EBiome[,] m_biomes = new EBiome[m_width, m_height];

    public int Width
    {
        get
        {
            return m_width;
        }
    }

    public int Height
    {
        get
        {
            return m_height;
        }
    }

    public EBiome this[int x, int y]
    {
        get
        {
            Debug.Assert(x >= 0 && x < Width);
            Debug.Assert(y >= 0 && y < Height);
            return m_biomes[x, y];
        }
        set
        {
            m_biomes[x, y] = value;
        }
    }

    public void Clear()
    {
        Array.Clear(m_biomes, 0, m_biomes.Length);
    }
}

public class BiomeManager
{
    private const int NumPoints = 256;

    List<Vector2> m_points;
    Texture m_debugTexture;
    BiomeMap m_biomeMap = new BiomeMap();

    /// <summary>
    /// How each Biome map to a generation template tells us how this area will be generated
    /// </summary>
    private Dictionary<EBiome, GenerationTemplate> m_biomeToGeneration = new Dictionary<EBiome, GenerationTemplate>(10);

    public BiomeMap Map
    {
        get
        {
            return m_biomeMap;
        }
    }

    public int Width
    {
        get
        {
            return Map.Width;
        }
    }

    public int Height
    {
        get
        {
            return Map.Height;
        }
    }

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
        plainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Forest, 0.02f, 0.0f));
        plainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Water, 0.02f, 0.0f));
        m_biomeToGeneration[EBiome.Plain] = plainTemplate;

        GenerationTemplate oceanTemplate = new GenerationTemplate();
        oceanTemplate.patchTemplate.Add(new PatchTemplate(ETile.Water, 1.0f, 1.0f));
        m_biomeToGeneration[EBiome.Ocean] = oceanTemplate;

        GenerationTemplate forestTemplate = new GenerationTemplate();
        forestTemplate.patchTemplate.Add(new PatchTemplate(ETile.Forest, 0.8f, 0.5f));
        m_biomeToGeneration[EBiome.Forest] = forestTemplate;

        GenerationTemplate desertTemplate = new GenerationTemplate();
        desertTemplate.patchTemplate.Add(new PatchTemplate(ETile.Desert, 0.95f, 0.80f));
        m_biomeToGeneration[EBiome.Desert] = desertTemplate;

        GenerationTemplate mountainTemplate = new GenerationTemplate();
        mountainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Mountain, 0.85f, 0.80f));
        mountainTemplate.patchTemplate.Add(new PatchTemplate(ETile.Mountain, 0.12f, 0.80f));
        m_biomeToGeneration[EBiome.Mountain] = mountainTemplate;
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
        if (m_points != null)
        {
            m_points.Clear();
        }
        m_debugTexture = null;
    }

    // Implement Voronoi tessellation 
    // https://en.wikipedia.org/wiki/Voronoi_diagram#/media/File:Voronoi_growth_euclidean.gif
    private void voronoiTesselate()
    {
        Stopwatch sw = Stopwatch.StartNew();

        int[,] regions = new int[Width, Height];
        for (int j = 0; j < Height; j++)
        {
            for (int i = 0; i < Width; i++)
            {
                regions[i, j] = -1;
            }
        }

        // Remap each of the point into the biome. It's possible some will overwrite each other, that's fine, shouldn't happen too much.
        Stopwatch swMapPoints = Stopwatch.StartNew();
        Dictionary<Tuple<int, int>, int> closePoints = new Dictionary<Tuple<int, int>, int>();
        int counter = 1;
        foreach (Vector2 point in m_points)
        {
            int x = (int)(point.x * Width);
            int y = (int)(point.y * Height);

            // Should be fine since point coordinates are supposed to be [0.0, 1.0[
            Debug.Assert(x >= 0 && x < Width);
            Debug.Assert(y >= 0 && y < Height);

            closePoints[new Tuple<int, int>(x, y)] = counter;
            regions[x, y] = counter;
            counter++;
        }
        swMapPoints.Stop();


        // time to fill the missing regions !
        // I can flood fill to figure out all the regions
        // or I can do pixel per pixel and figure out which point is the closest
        // flood fill if done properly will be have run-time of O(Width * Height) + use some memory and is a bit more complex
        // while pixel per pixel will have O(Width * Height * NumPoints) and is simple.
        // So we will do pixel per pixel for now, and we can optimize later
        Stopwatch minDistance = Stopwatch.StartNew();
        for (int j = 0; j < Height; j++)
        {
            for (int i = 0; i < Width; i++)
            {
                if (regions[i,j] != -1) // already done
                {
                    continue;
                }

                // not done yet, find the closest point
                KeyValuePair<Tuple<int, int>, int> best;
                float bestSqrDistance = float.MaxValue;
                foreach (var candidate in closePoints)
                {
                    Vector2 firstValue = new Vector2(i, j);
                    Vector2 secondValue = new Vector2(candidate.Key.Item1, candidate.Key.Item2);
                    float sqrDistance = (firstValue - secondValue).sqrMagnitude;

                    if (sqrDistance < bestSqrDistance)
                    {
                        bestSqrDistance = sqrDistance;
                        best = candidate;
                    }
                }

                regions[i, j] = best.Value;
            }
        }
        minDistance.Stop();

        // For now just map all region back to EBiome
        Stopwatch remap = Stopwatch.StartNew();
        for (int j = 0; j < Height; j++)
        {
            for (int i = 0; i < Width; i++)
            {
                int value = regions[i, j];
                value %= Enum.GetNames(typeof(EBiome)).Length;
                Map[i, j] = (EBiome)value;
            }
        }
        remap.Stop();

        GenerateDebugTexture();
        sw.Stop();
        Debug.Log(string.Format("BiomeManager: Voronoi tessellation took {0} ms", sw.ElapsedMilliseconds));
        Debug.Log(string.Format("BiomeManager: Voronoi tessellation remapPoints took {0} ms", swMapPoints.ElapsedMilliseconds));
        Debug.Log(string.Format("BiomeManager: Voronoi tessellation min distance took {0} ms", minDistance.ElapsedMilliseconds));
        Debug.Log(string.Format("BiomeManager: Voronoi tessellation remap took {0} ms", remap.ElapsedMilliseconds));
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
