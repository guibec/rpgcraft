#include "PCH-rpgcraft.h"
#include "Box2D.h"

Box2D::~Box2D()
{
}

void Box2D::buildFrom(float2 origin, float halfWidth, float halfHeight)
{
	m_topLeft = float2(origin.x - halfWidth, origin.y + halfHeight);
	m_bottomRight = float2(origin.x + halfWidth, origin.y - halfHeight);
	normalize();
}

void Box2D::normalize()
{
    if (left() > right())
    {
        float temp = left();
        setLeft(right());
        setRight(temp);
    }

    if (bottom() > top())
    {
        float temp = bottom();
        setBottom(top());
        setTop(temp);
    }
}
