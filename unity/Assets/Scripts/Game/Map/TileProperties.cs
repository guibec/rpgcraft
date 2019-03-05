public class TileProperties
{
    /// <summary>
    /// Impact to the speed of characters when they are crossing this tile
    /// </summary>
    public float SpeedFactor { get; set; } = 1.0f;

    /// <summary>
    /// Can the tile be crossed by movements
    /// </summary>
    public bool IsPassable { get; set; } = true;

    /// <summary>
    /// How much health the tile has for mining.
    /// </summary>
    public int Durability { get; set; } = 100;

}
