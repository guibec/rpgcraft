using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using SimpleJSON;

public struct TileDef
{
    public TileDef(ETile id, string name, TileResourceDef tileResourceDef) : this()
    {
        Id = id;
        Name = name;
        Resource = tileResourceDef;
    }

    public ETile Id { private set; get; }
    public string Name { private set; get; }
    public TileResourceDef Resource { private set; get; }
}

public struct TileResourceDef
{
    public TileResourceDef(string filename, Rect rect, int count) : this()
    {
        Filename = filename;
        Rect = rect;
        Count = count;
    }

    public string Filename { private set; get; }
    public Rect Rect { private set; get; }
    public int Count { private set; get; }
}


public static class TileMapping
{
    private static Dictionary<ETile, TileDef> m_tilesDef = new Dictionary<ETile, TileDef>(50);

    static public TileResourceDef? GetTileResourceDef(ETile tile)
    {
        TileDef tileDef;
        if (!m_tilesDef.TryGetValue(tile, out tileDef))
            return null;
        else
        {
            return tileDef.Resource;
        }
    }

    public static bool BuildFromJSON(string filename)
    {
        TextAsset tileInfo = Resources.Load(filename) as TextAsset;
        if (tileInfo != null)
        {
            JSONNode rootNode = JSON.Parse(tileInfo.text);
            return TileMapping.BuildFromJSON(rootNode);
        }

        return false;
    }

    private static bool BuildFromJSON(JSONNode rootNode)
    {
        m_tilesDef.Clear();

        JSONNode tilesInfo = rootNode["tilesInfo"];
        if (tilesInfo == null)
            return false;

        foreach (var tileNode in tilesInfo.Childs)
        {
            JSONNode resNode = tileNode["resource"];
            if (resNode == null)
                continue;

            string filename = resNode["file"];
            int x = resNode["x"].AsInt;
            int y = resNode["y"].AsInt;
            int w = resNode["w"].AsInt;
            int h = resNode["h"].AsInt;
            int count = resNode["count"].AsInt;

            TileResourceDef trd = new TileResourceDef(filename, new Rect(x, y, w, h), count);

            ETile id = (ETile)tileNode["id"].AsInt;
            string name = tileNode["name"];

            TileDef td = new TileDef(id, name, trd);

            if (m_tilesDef.ContainsKey(id))
            {
                Debug.Log(string.Format("Ignoring duplicate id {0}", id));
                continue;
            }

            

            m_tilesDef.Add(id, td);
        }

        return true;
    }

    public static void GetUVFromTile(TileInfo tileInfo, out Vector2 ul, out Vector2 ur, out Vector2 bl, out Vector2 br)
    {
        TileDef tileDef;

        if (m_tilesDef.TryGetValue(tileInfo.Tile, out tileDef))
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
            ul = new Vector2(tileDef.Resource.Rect.xMin + pixelOffset, tileDef.Resource.Rect.yMin);
            ur = new Vector2(tileDef.Resource.Rect.xMax + pixelOffset, tileDef.Resource.Rect.yMin);
            bl = new Vector2(tileDef.Resource.Rect.xMin + pixelOffset, tileDef.Resource.Rect.yMax);
            br = new Vector2(tileDef.Resource.Rect.xMax + pixelOffset, tileDef.Resource.Rect.yMax);
            
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
