using UnityEngine;
using System.Collections.Generic;
using System.Linq;

[System.Serializable]
public class FrameInfo
{
    public int m_frameIndex;
    public Rect m_rect;
    public FrameInfo m_nextFrame;
    public FrameInfo m_previousFrame;
}

[System.Serializable]
public class FrameGroup
{
    public FrameGroup()
    {
    }

    public FrameGroup(string name)
    {
        m_name = name;
    }

    //[Inspect]
    public string m_name;

    //[Inspect]
    public List<FrameInfo> m_frames = new List<FrameInfo>(3);
}

[System.Serializable]
//public class FrameGroupDictionary : UDictionary<string, FrameGroup>
public class FrameGroupDictionary : Dictionary<string, FrameGroup>
{
}

public class EntityRender : MonoBehaviourEx 
{
    public Texture m_texture;

    //[Inspect]
    public FrameGroupDictionary m_frameGroups = new FrameGroupDictionary();

    private MeshRenderer m_mr;
    private Mesh m_mesh;

    /// <summary>
    /// delay between each frame
    /// </summary>
    private float m_globalFrameDelay;

    /// <summary>
    /// last timestamp a frame was set
    /// </summary>
    private float m_consumedDt;

    // run-time
    private FrameGroup m_currentFrameGroup;

    private int m_currentFrameIndex;
    private FrameInfo m_currentFrame;

    // Use this for initialization
    void Awake () 
    {
        m_mr = gameObject.GetComponent<MeshRenderer>();
        m_mesh = GetComponent<MeshFilter>().mesh;
    }

    void Start()
    {
        if (m_mr)
        {
            m_mr.material.mainTexture = m_texture;
        }
    }
    
    // Update is called once per frame
    void Update ()
    {
        m_consumedDt += TimeManager.Dt;

        if (m_currentFrameGroup != null)
        {
            // If there are multiple Entity renderers, the last one win.
            if (m_mr)
            {
                m_mr.material.mainTexture = m_texture;
            }

            while (m_consumedDt >= m_globalFrameDelay)
            {
                if (m_currentFrame?.m_nextFrame != null)
                {
                    int newIndex = m_currentFrame.m_nextFrame.m_frameIndex;
                    ChangeFrame(newIndex);
                    m_consumedDt -= m_globalFrameDelay;
                }
                else
                {
                    break;
                }
            }
        }
    }

    public void SetFrameInfo(string groupName, int px, int py, int width, int height)
    {
        // add/update from dictionary
        if (!m_frameGroups.TryGetValue(groupName, out var frameGroup))
        {
            frameGroup = new FrameGroup(groupName);
            m_frameGroups.Add(groupName, frameGroup);
        }

        Rect rect = new Rect(px, py, width, height);
        FrameInfo fi = new FrameInfo
        {
            m_rect = rect,
            m_frameIndex = frameGroup.m_frames.Count
        };
        frameGroup.m_frames.Add(fi);
    }

    public bool LinkNextFrame(string group, int frame, int nextFrame)
    {
        if (!m_frameGroups.TryGetValue(group, out var frameGroup))
        {
            return false;
        }

        FrameInfo curFrame = frameGroup.m_frames.ElementAt(frame);
        FrameInfo otherFrame = frameGroup.m_frames.ElementAt(nextFrame);

        curFrame.m_nextFrame = otherFrame;
        otherFrame.m_previousFrame = curFrame;

        return true;
    }

    public void SetGlobalFrameDelay(float delay)
    {
        m_globalFrameDelay = delay;
    }

    public string GetCurrentGroup()
    {
        return m_currentFrameGroup != null ? m_currentFrameGroup.m_name : "";
    }

    public bool SetCurrentGroup(string group)
    {
        if (m_currentFrameGroup == null || m_currentFrameGroup.m_name != group)
        {
            if (!m_frameGroups.TryGetValue(group, out m_currentFrameGroup))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        InitFrame(0);
        return true;
    }

    private void InitFrame(int frameIndex)
    {
        ChangeFrame(frameIndex);
        m_consumedDt = 0.0f;
    }

    private void ChangeFrame(int frameIndex)
    {
        m_currentFrameIndex = frameIndex;
        m_currentFrame = m_currentFrameGroup.m_frames.ElementAt(m_currentFrameIndex);

        SetUVs();
        
    }

    private void SetUVs()
    {
        if (m_currentFrame == null)
            return;

        Vector2[] uvs = m_mesh.uv;

        int w = m_texture.width;
        int h = m_texture.height;

        uvs[0] = new Vector2(m_currentFrame.m_rect.xMin / w, m_currentFrame.m_rect.yMin / h);
        uvs[1] = new Vector2(m_currentFrame.m_rect.xMax / w, m_currentFrame.m_rect.yMax / h);
        uvs[2] = new Vector2(m_currentFrame.m_rect.xMax / w, m_currentFrame.m_rect.yMin / h);
        uvs[3] = new Vector2(m_currentFrame.m_rect.xMin / w, m_currentFrame.m_rect.yMax / h);

        m_mesh.uv = uvs;
    }
}
