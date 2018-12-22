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

    private readonly EBiome[,] m_biomes = new EBiome[Width, Height];

    List<Vector2> m_points;

    Mesh m_debugMesh;
    Material m_debugMaterial;

    public BiomeManager()
    {
        // 32 biome types
        const int numPoints = 32;

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
        Array.Clear(regions, 0, regions.Length);

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

            closePoints.Add(new Tuple<int, int>(x, y), counter);
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
                if (regions[i,j] != 0) // already done
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

    public Mesh GetDebugMesh()
    {
        return m_debugMesh;
    }
}
