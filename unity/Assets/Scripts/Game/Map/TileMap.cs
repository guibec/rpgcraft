using System;
using UnityEngine;

[RequireComponent(typeof(MeshFilter))]
[RequireComponent(typeof(MeshRenderer))]
[ExecuteInEditMode]
public class TileMap : MonoBehaviourEx
{
    // if true, m_width and m_height are set automatically
    public bool m_autoSize = false; 
    public int m_width = 8;     // number of horizontal tiles
    public int m_height = 8;    // number of vertices tiles
    public int m_tileSize = 16;   // tile pixel size (square)

    private Vector3? m_currentSelection;

    private bool m_dirty;

    // Use this for initialization
    void Start () 
    {
        Regenerate();
    }

    public Vector3? SelectedTile
    {
        get
        {
            return m_currentSelection;
        }
        set
        {
            m_currentSelection = value;
        }
    }

    private ChunkInfo m_sourceChunk;
    public ChunkInfo SourceChunk
    {
        get
        {
            return m_sourceChunk;
        }
        set
        {
            if (m_sourceChunk != value)
            {
                if (m_sourceChunk != null)
                    m_sourceChunk.Changed -= OnSourceChunkChanged;
                
                m_sourceChunk = value;
                m_sourceChunk.Changed += OnSourceChunkChanged;

                OnSourceChunkUpdated();
            }
        }
    }

    public Vector3 FromWorldToTile(Vector3 input)
    {
        return new Vector3((int)input.x + m_width / 2, (int)input.y + m_height / 2);
    }

    private void UpdateUVs()
    {
        int numVertices = (m_width) * (m_height) * 4;
        Vector2[] uvs = new Vector2[numVertices];

        if (SourceChunk != null)
        {
            for (int j = 0; j < m_height; ++j)
            {
                for (int i = 0; i < m_width; ++i)
                {
                    TileInfo tileInfo = m_sourceChunk.ReadSlotValue(i, j);
                    int vertexIndex = (j * m_width + i) * 4;
                    TileMapping.Instance.GetUVFromTile(tileInfo, out uvs[vertexIndex], out uvs[vertexIndex + 1], out uvs[vertexIndex + 2], out uvs[vertexIndex + 3]);
                }
            }
        }

        MeshFilter mf = GetComponent<MeshFilter>();
        mf.mesh.uv = uvs;

        m_dirty = false;
    }

    public void Update()
    {
        if (m_dirty)
            UpdateUVs();
    }

    public void Regenerate()
    {
        // Assign the unique atlas texture
        MeshRenderer meshRenderer = GetComponent<MeshRenderer>();
        meshRenderer.material.mainTexture = TileMapping.Instance.AtlasTexture;

        BuildMesh();
    }

    private void OnSourceChunkUpdated()
    {
        UpdateUVs();
    }

    private void OnSourceChunkChanged(object sender, EventArgs e)
    {
        m_dirty = true;
    }

    private void BuildMesh()
    {
        if (m_autoSize)
        {
            MeshRenderer mr = GetComponent<MeshRenderer>();

            int texWidth = mr.material.mainTexture.width;
            int texHeight = mr.material.mainTexture.height;

            m_width = texWidth / m_tileSize;
            m_height = texHeight / m_tileSize;
        }

        Shader shader = Shader.Find("dig");
        if (shader == null)
        {
            Debug.Log("Could not find shader");
        }

        int numVertices = m_width*m_height*4;
        int numQuad = m_width * m_height;

        Vector3 offset = new Vector3(-m_width / 2f, -m_height / 2f, 0.0f);

        // setup vertices and normals
        Vector3[] vertices = new Vector3[numVertices];
        Vector3[] normals = new Vector3[numVertices];
        for (int j = 0; j < m_height; ++j)
        {
            for (int i = 0; i < m_width; ++i)
            {
                int vertexIndex = (j*m_width + i)*4;
                vertices[vertexIndex] = new Vector3(offset[0] + i, offset[1] + j, offset[2]);
                vertices[vertexIndex+1] = new Vector3(offset[0] + i + 1, offset[1] + j, offset[2]);
                vertices[vertexIndex+2] = new Vector3(offset[0] + i, offset[1] + j + 1, offset[2]);
                vertices[vertexIndex+3] = new Vector3(offset[0] + i + 1, offset[1] + j + 1, offset[2]);

                normals[vertexIndex] = new Vector3(0, 0, 1.0f);
                normals[vertexIndex+1] = new Vector3(0, 0, 1.0f);
                normals[vertexIndex+2] = new Vector3(0, 0, 1.0f);
                normals[vertexIndex+3] = new Vector3(0, 0, 1.0f);
            }
        }

        // setup triangle indices
        int[] indices = new int[numQuad * 2 * 6];
        for (int j = 0; j < m_height; ++j)
        {
            for (int i = 0; i < m_width; ++i)
            {
                int indexOffset = (j * m_width + i) * 6;
                int vertexIndex = (j * m_width + i) * 4;

                // each quad is made up of 2 triangles
                int topLeft     = vertexIndex;
                int topRight    = vertexIndex + 1;
                int bottomLeft  = vertexIndex + 2;
                int bottomRight = vertexIndex + 3;

                indices[indexOffset + 0] = topLeft;
                indices[indexOffset + 1] = bottomLeft;
                indices[indexOffset + 2] = bottomRight;
                indices[indexOffset + 3] = topLeft;
                indices[indexOffset + 4] = bottomRight;
                indices[indexOffset + 5] = topRight;
            }
        }


        Mesh mesh = new Mesh {vertices = vertices, normals = normals, triangles = indices};

        MeshFilter mf = GetComponent<MeshFilter>();
        mf.mesh = mesh;

        MeshCollider mc = GetComponent<MeshCollider>();
        if (mc)
            mc.sharedMesh = mesh;

        UpdateUVs();
    }
    
}

