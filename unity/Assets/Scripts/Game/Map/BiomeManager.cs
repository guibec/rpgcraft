using System;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;

using IronExtension;
using Debug = UnityEngine.Debug;

public class BiomeManager
{
    private const int Width = 256;
    private const int Height = 256;
    private const int NumPoints = 64;

    private readonly EBiome[,] m_biomes = new EBiome[Width, Height];

    List<Vector2> m_points;
    Texture m_debugTexture;

    public EBiome[,] BiomeMap
    {
        get
        {
            return m_biomes;
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
        m_points = new List<Vector2>(Width * Height);

        for (int i = 0; i < NumPoints; i++)
        {
            Vector2 point = new Vector2(RandomManager.Next(0, 1.0f), RandomManager.Next(0, 1.0f));
            m_points.Add(point);
        }

        voronoiTesselate();
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

        // time to fill the missing regions !
        // I can flood fill to figure out all the regions
        // or I can do pixel per pixel and figure out which point is the closest
        // flood fill if done properly will be have run-time of O(Width * Height) + use some memory and is a bit more complex
        // while pixel per pixel will have O(Width * Height * NumPoints) and is simple.
        // So we will do pixel per pixel for now, and we can optimize later

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

        // For now just map all region back to EBiome
        for (int j = 0; j < Height; j++)
        {
            for (int i = 0; i < Width; i++)
            {
                int value = regions[i, j];
                value %= Enum.GetNames(typeof(EBiome)).Length;
                m_biomes[i, j] = (EBiome)value;
            }
        }

        GenerateDebugTexture();
        sw.Stop();
        Debug.Log(string.Format("BiomeManager: Voronoi tessellation took {0} ms", sw.ElapsedMilliseconds));
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

                switch (m_biomes[i,j])
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
                    case EBiome.Snow:
                        pixelColor = Color.white;
                        break;
                    case EBiome.Jungle:
                        pixelColor = Color.magenta;
                        break;
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
