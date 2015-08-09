using UnityEngine;
using System.Collections;

public struct Box2D 
{
    public Box2D(Vector2 origin, float halfWidth, float halfHeight) : this()
    {
        BuildFrom(origin, halfWidth, halfHeight);
        Normalize();
    }

    public Box2D(Vector2 topLeft, Vector2 bottomRight)
    {
        m_topLeft = topLeft;
        m_bottomRight = bottomRight;
        Normalize();
    }

    public Box2D(float left, float top, float right, float bottom)
        : this(new Vector2(left, top), new Vector2(right, bottom))
    {
        Normalize();
    }

    private void BuildFrom(Vector2 origin, float halfWidth, float halfHeight)
    {
        m_topLeft       = new Vector2(origin.x - halfWidth, origin.y + halfHeight);
        m_bottomRight   = new Vector2(origin.x + halfWidth, origin.y - halfHeight);
    }

    private void Normalize()
    {
        if (Left > Right)
        {
            float temp = Left;
            Left = Right;
            Right = temp;
        }

        if (Bottom > Top)
        {
            float temp = Bottom;
            Bottom = Top;
            Top = temp;
        }
    }

    public Vector2 Center
    {
        get
        {
            return ((m_topLeft + m_bottomRight) / 2);
        }
        set
        {
            BuildFrom(value, Width/2, Height/2);
        }
    }

    public float Width
    {
        get
        {
            return m_bottomRight.x - m_topLeft.x;
        }
        set
        {
            BuildFrom(Center, Width/2, Height);
        }
    }

    public float Height
    {
        get
        {
            return m_topLeft.y - m_bottomRight.y;
        }
        set
        {
            BuildFrom(Center, Width, Height/2);
        }
    }

    public float Left
    {
        get
        {
            return m_topLeft.x;
        }
        set
        {
            m_topLeft.x = value;
        }
    }

    public float Right
    {
        get
        {
            return m_bottomRight.x;
        }
        set
        {
            m_bottomRight.x = value;
        }
    }

    public float Top
    {
        get
        {
            return m_topLeft.y;
        }
        set
        {
            m_topLeft.y = value;
        }
    }

    public float Bottom
    {
        get
        {
            return m_bottomRight.y;
        }
        set
        {
            m_bottomRight.y = value;
        }
    }

    public void Draw(Color c)
    {
        DebugUtils.DrawRect(m_topLeft, m_bottomRight, c);
    }

    // These accessors are added to make it easier to index the Box
    public float Min(int index)
    {
        DebugUtils.Assert(index >= 0 && index <= 1);

        if (index == 0)
            return m_topLeft.x;
        else
            return m_bottomRight.y;
    }

    public float Max(int index)
    {
        DebugUtils.Assert(index >= 0 && index <= 1);

        if (index == 0)
            return m_bottomRight.x;
        else
            return m_topLeft.y;
    }

    private Vector2 m_topLeft;
    private Vector2 m_bottomRight;
}
