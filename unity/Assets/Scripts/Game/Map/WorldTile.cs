﻿public static class WorldTile
{
    /* next entry here should add attributes to tiles instead.
    // suggest using the JSON file
    // "Water" : { "Properties" : [ "Impassable", "" ] }
    */ 
    public static bool IsCollision(ETile tile)
    {
        return tile == ETile.Mountain || tile == ETile.Invalid;
    }
}
