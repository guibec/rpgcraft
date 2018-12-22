using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BiomeManager
{
    private const int Width = 64;
    private const int Height = 64;

    private readonly EBiome[,] biomes = new EBiome[Width, Height];

    List<Vector2> m_points;

    Mesh m_debugMesh;
    Material m_debugMaterial;

    public BiomeManager()
    {
        // Width * Height points randomly on the [0.0 to 1.0 range]
        const int numPoints = Width * Height;

        m_points = new List<Vector2>(Width * Height);

        for (int i = 0; i < numPoints; i++)
        {
            Vector2 point = new Vector2(RandomManager.Next(0, 1.0f), RandomManager.Next(0, 1.0f));
            m_points.Add(point);
        }

        m_debugMesh = CreatePointMesh(m_points.ToArray().toVector3Array());
        m_debugMaterial = new Material(Shader.Find("Sprites/Default"));

        // Then we apply Fortune's algorithm
        // https://en.wikipedia.org/wiki/Fortune's_algorithm
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
