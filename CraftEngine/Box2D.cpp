#include "PCH-rpgcraft.h"
#include "Box2D.h"

Box2D::~Box2D()
{
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
