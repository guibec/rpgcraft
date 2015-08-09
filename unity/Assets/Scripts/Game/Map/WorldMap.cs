using UnityEngine;
using System.Collections;

public static class WorldTile
{
    public static bool IsCollision(ETile tile)
    {
        return tile == ETile.Mountain || tile == ETile.Invalid;
    }
}
