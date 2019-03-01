using UnityEngine;
using System.Collections.Generic;
using TileData;

namespace TileData
{
    public class TileResourceDef
    {
        public string Filename { set; get; }
        public Rect Rect { set; get; }
        public int Count { set; get; }
    }

    public class TileDef
    {
        public string Name { get; set; }
        public TileResourceDef Resource { get; set; }

        public TileProperties Properties { get; set; }
    }

    public class TilesInfo
    {
        public string version;
        public Dictionary<ETile, TileDef> tilesInfo = new Dictionary<ETile, TileDef>(50);
        public Dictionary<ETile, ETile> mining = new Dictionary<ETile, ETile>(50);
    }
}

public static class TileMapping
{
    private static TilesInfo m_tilesInfo;

    public static TileResourceDef GetTileResourceDef(ETile tile)
    {
        return !m_tilesInfo.tilesInfo.TryGetValue(tile, out var tileDef) ? null : tileDef.Resource;
    }

    public static TileProperties GetTileProperties(ETile tile)
    {
        return !m_tilesInfo.tilesInfo.TryGetValue(tile, out var tileDef) ? null : tileDef.Properties;
    }

    public static bool BuildFromJSON(string filename)
    {
        m_tilesInfo = JSONUtils.LoadJSON<TilesInfo>(filename);
        return m_tilesInfo != null;
    }

    /// <summary>
    /// Return the new ETile from input after it has been mined. If input cannot be mined, return Invalid
    /// </summary>
    /// <param name="input">The tile to mine</param>
    /// <returns></returns>
    public static ETile GetMiningTransform(ETile input)
    {
        return m_tilesInfo.mining.TryGetValue(input, out ETile result) ? result : ETile.Invalid;
    }
    
    public static void GetUVFromTile(TileInfo tileInfo, out Vector2 ul, out Vector2 ur, out Vector2 bl, out Vector2 br)
    {
        if (m_tilesInfo.tilesInfo.TryGetValue(tileInfo.Tile, out var tileDef))
        {
            int countOffset = 0;
            if (tileDef.Resource.Count > 0)
            {
                if (tileInfo.HP == 100f)
                    countOffset = 0;
                else if (tileInfo.HP >= 90f)
                    countOffset = 1;
                else if (tileInfo.HP >= 80f)
                    countOffset = 2;
                else if (tileInfo.HP >= 70f)
                    countOffset = 3;
                else if (tileInfo.HP >= 60f)
                    countOffset = 4;
                else if (tileInfo.HP >= 50f)
                    countOffset = 5;
                else if (tileInfo.HP >= 40f)
                    countOffset = 6;
                else if (tileInfo.HP >= 30f)
                    countOffset = 7;
                else if (tileInfo.HP >= 20f)
                    countOffset = 8;
                else if (tileInfo.HP >= 10f)
                    countOffset = 9;
                else if (tileInfo.HP >= 0f)
                    countOffset = 10;

            }

            float pixelOffset = countOffset*tileDef.Resource.Rect.width;
            bl = new Vector2(tileDef.Resource.Rect.xMin + pixelOffset, tileDef.Resource.Rect.yMin);
            br = new Vector2(tileDef.Resource.Rect.xMax + pixelOffset, tileDef.Resource.Rect.yMin);
            ul = new Vector2(tileDef.Resource.Rect.xMin + pixelOffset, tileDef.Resource.Rect.yMax);
            ur = new Vector2(tileDef.Resource.Rect.xMax + pixelOffset, tileDef.Resource.Rect.yMax);
            
            // now remap to real UVs and invert Y since I like to count from top to bottom for tile indices
            const float textureSize = 256f;
            ul = new Vector2(ul.x / textureSize, (textureSize - ul.y) / textureSize);
            ur = new Vector2(ur.x / textureSize, (textureSize - ur.y) / textureSize);
            bl = new Vector2(bl.x / textureSize, (textureSize - bl.y) / textureSize);
            br = new Vector2(br.x / textureSize, (textureSize - br.y) / textureSize);
        }
        else
        {
            ul = new Vector2(0, 0);
            ur = new Vector2(0, 0);
            bl = new Vector2(0, 0);
            br = new Vector2(0, 0);
        }
    }
}
