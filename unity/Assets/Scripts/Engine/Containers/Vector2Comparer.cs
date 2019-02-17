using UnityEngine;
using System.Collections.Generic;

class Vector2Comparer : IComparer<Vector2>, IEqualityComparer<Vector2>
{
    public int Compare(Vector2 a, Vector2 b)
    {
        if (a.y < b.y)
            return -1;
        if (a.y == b.y)
        {
            if (a.x == b.x)
                return 0;
            if (a.x < b.x)
                return -1;
        }
        return 1;
    }

    public bool Equals(Vector2 x, Vector2 y)
    {
        return Compare(x, y) == 0;
    }

    public int GetHashCode(Vector2 obj)
    {
        int hash = 17;
        hash = hash*29 + obj.x.GetHashCode();
        hash = hash*29 + obj.y.GetHashCode();
        return hash;
    }
}    

