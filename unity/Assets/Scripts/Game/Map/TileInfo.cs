using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

// Next time another entry is added here, we need to make this enum self-generated
public enum ETile
{
    Invalid = 0,
    Grass,
    Dirt,
    Mountain,
    Desert,
    Tree,
    Forest,
    Water,
}

// describe a specific tile
public struct TileInfo
{
    public ETile Tile { get; private set; }
    public float HP { get; private set; }

    private static readonly TileInfo Invalid = new TileInfo(
        ETile.Invalid);

    public static implicit operator TileInfo(ETile tile_)
    {
        return new TileInfo(tile_);
    }

    public TileInfo(ETile tile_) : this()
    {
        if (tile_ == ETile.Mountain || tile_ == ETile.Tree)
            HP = 100.0f;
        else
            HP = 0.0f;

        Tile = tile_;
    }

    public TileInfo(ETile tile_, float hp_) : this()
    {
        Tile = tile_;
        HP = hp_;
    }

    public TileInfo TransformToTile(ETile tile_)
    {
        TileInfo newTileInfo = new TileInfo(tile_, HP);
        return newTileInfo;
    }

    public TileInfo RemoveHP(float hp)
    {
        float newHp = HP - hp;
        newHp = Math.Max(newHp, 0.0f);

        TileInfo newTileInfo = new TileInfo(Tile, newHp);
        return newTileInfo;
    }

    public TileInfo MaxHP()
    {
        float newHp;
        if (Tile == ETile.Mountain)
            newHp = 100.0f;
        else
            newHp = 0.0f;

        TileInfo newTileInfo = new TileInfo(Tile, newHp);
        return newTileInfo;
    }

    static public TileInfo GetInvalid()
    {
        return Invalid;
    }

    public override string ToString()
    {
        return base.ToString() + " " + Tile.ToString();
    }
}
