using UnityEngine;
using System.Collections;

public static class TileMapping
{
    public static void GetUVFromTile(TileInfo tileInfo, out Vector2 ul, out Vector2 ur, out Vector2 bl, out Vector2 br)
    {
        // Hardcoded stuff for now
        Vector2 result = new Vector2(0f, 0f);
        switch (tileInfo.Tile)
        {
            case ETile.Desert:
                result = new Vector2(2, 1);
                break;
            case ETile.Dirt:
                result = new Vector2(2, 0);
                break;
            case ETile.Forest:
                result = new Vector2(15, 0);
                break;
            case ETile.Grass:
                result = new Vector2(0, 0);
                break;
            case ETile.Mountain:
                {
                    if (tileInfo.HP == 100.0f)
                        result = new Vector2(13, 2);
                    else if (tileInfo.HP > 90.0f)
                        result = new Vector2(0, 15);
                    else if (tileInfo.HP > 80.0f)
                        result = new Vector2(1, 15);
                    else if (tileInfo.HP > 70.0f)
                        result = new Vector2(2, 15);
                    else if (tileInfo.HP > 60.0f)
                        result = new Vector2(3, 15);
                    else if (tileInfo.HP > 50.0f)
                        result = new Vector2(4, 15);
                    else if (tileInfo.HP > 40.0f)
                        result = new Vector2(5, 15);
                    else if (tileInfo.HP > 30.0f)
                        result = new Vector2(6, 15);
                    else if (tileInfo.HP > 20.0f)
                        result = new Vector2(7, 15);
                    else if (tileInfo.HP > 10.0f)
                        result = new Vector2(8, 15);
                    else if (tileInfo.HP >= 0.0f)
                        result = new Vector2(9, 15);



                }
                break;
            case ETile.Tree:
                result = new Vector2(15, 5);
                break;
        }

        // now remap to real UVs and invert Y since I like to count from top to bottom for UVs
        ul = new Vector2((result.x + 0) * 16f / 256f, 256f - (result.y + 1) * 16f / 256f);
        ur = new Vector2((result.x + 1) * 16f / 256f, 256f - (result.y + 1) * 16f / 256f);
        bl = new Vector2((result.x + 0) * 16f / 256f, 256f - (result.y + 0) * 16f / 256f);
        br = new Vector2((result.x + 1) * 16f / 256f, 256f - (result.y + 0) * 16f / 256f);
    }
}
