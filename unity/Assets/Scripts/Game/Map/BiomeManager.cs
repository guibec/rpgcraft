using System;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;

using IronExtension;
using Debug = UnityEngine.Debug;

public class BiomeManager
{
    private const int Width = 64;
    private const int Height = 64;
    private const int NumPoints = 32;

    private readonly EBiome[,] m_biomes = new EBiome[Width, Height];

    List<Vector2> m_points;

    Mesh m_debugMesh;
    Material m_debugMaterial;
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

        for (int i = 0; i < numPoints; i++)
        {
            Vector2 point = new Vector2(RandomManager.Next(0, 1.0f), RandomManager.Next(0, 1.0f));
            m_points.Add(point);
        }

        m_debugMesh = CreatePointMesh(m_points.ToArray().ToVector3Array());
        m_debugMaterial = new Material(Shader.Find("Sprites/Default"));

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
        for (int j = 0; j < 64; j++)
        {
            for (int i = 0; i < 64; i++)
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

    private Mesh CreatePointMesh(Vector3[] points)
    {
        Mesh mesh = new Mesh();
        mesh.vertices = points;
        // You can also apply UVs or vertex colors here.

        int[] indices = new int[points.Length];
        for (int i = 0; i < points.Length; i++)
            indices[i] = i;

        mesh.SetIndices(indices, MeshTopology.Points, 0);

        return mesh;
    }

    public void Update()
    {
        Transform playerTransform = GameManager.Instance.MainPlayer.transform;

        Vector3 scale = new Vector3(128.0f, 128.0f, 1.0f);
        Vector3 pos = new Vector3(-64.0f, -64.0f, 0.0f);
        Matrix4x4 matrix = Matrix4x4.TRS(pos, Quaternion.identity, scale);

        Graphics.DrawMesh(m_debugMesh, matrix, m_debugMaterial, 0);
    }

    private void GenerateDebugTexture()
    {
        // Create a new 2x2 texture ARGB32 (32 bit with alpha) and no mipmaps
        var texture = new Texture2D(64, 64, TextureFormat.ARGB32, false);

        for (int j = 0; j < 64; j++)
        {
            for (int i = 0; i < 64; i++)
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

    public Mesh GetDebugMesh()
    {
        return m_debugMesh;
    }
}
