public static class WorldTile
{
    public static bool IsCollision(ETile tile)
    {
        return !TileMapping.GetTileProperties(tile).IsPassable;
    }
}
