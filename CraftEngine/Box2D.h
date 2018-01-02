#pragma once
class Box2D
{
public:
    Box2D(float2 topLeft, float2 bottomRight) 
        : m_topLeft(topLeft)
        , m_bottomRight(bottomRight)
    {
        normalize();
    }

	Box2D(float2 origin, float halfWidth, float halfHeight)
	{
		buildFrom(origin, halfWidth, halfHeight);
		normalize();
	}

	Box2D(float left, float top, float right, float bottom)
		: m_topLeft(left, top), m_bottomRight(right, bottom)
	{
		normalize();
	}

    ~Box2D();

    float left() const { return m_topLeft.x; }
    float right() const { return m_bottomRight.x; }
    float top() const { return m_topLeft.y; }
    float bottom() const { return m_bottomRight.y; }

    void setLeft(float value) {
        m_topLeft.x = value;
    }

    void setRight(float value) {
        m_bottomRight.x = value;
    }

    void setTop(float value) {
        m_topLeft.y = value;
    }

    void setBottom(float value) {
        m_bottomRight.y = value;
    }

    // These accessors are added to make it easier to index the Box
    float min(int index) const
    {
        //DebugUtils.Assert(index >= 0 && index <= 1);

        if (index == 0)
            return m_topLeft.x;
        else
            return m_bottomRight.y;
    }

    float max(int index) const
    {
        //DebugUtils.Assert(index >= 0 && index <= 1);

        if (index == 0)
            return m_bottomRight.x;
        else
            return m_topLeft.y;
    }

private:

	void buildFrom(float2 origin, float halfWidth, float halfHeight);
    void normalize();

    float2 m_topLeft, m_bottomRight;
};

