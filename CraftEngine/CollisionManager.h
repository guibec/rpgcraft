#pragma once

#include "Box2D.h"

class CollisionManager
{
public:
    CollisionManager();
    ~CollisionManager();

    static bool testBox2DBox2D(Box2D a, Box2D b)
    {
        // Exit with no intersection if separated along an axis
        if (a.max(0) < b.min(0) || a.min(0) > b.max(0)) {
            return false;
        }

        if (a.max(1) < b.min(1) || a.min(1) > b.max(1)) {
            return false;
        }

        // Overlapping on all axes means AABBs are intersecting
        return true;
    }
};

