using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using SimpleJSON;

struct TileDef
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

struct TileResourceDef
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
    private static List<TileDef> m_tilesDef = new List<TileDef>(50);

    public static bool BuildFromJSON(string filename)
    {
        m_tilesDef.Clear();

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

            m_tilesDef.Add(td);
        }

        return true;
    }

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
                        result = new Vector2(0, 15);
                    else if (tileInfo.HP > 90.0f)
                        result = new Vector2(1, 15);
                    else if (tileInfo.HP > 80.0f)
                        result = new Vector2(2, 15);
                    else if (tileInfo.HP > 70.0f)
                        result = new Vector2(3, 15);
                    else if (tileInfo.HP > 60.0f)
                        result = new Vector2(4, 15);
                    else if (tileInfo.HP > 50.0f)
                        result = new Vector2(5, 15);
                    else if (tileInfo.HP > 40.0f)
                        result = new Vector2(6, 15);
                    else if (tileInfo.HP > 30.0f)
                        result = new Vector2(7, 15);
                    else if (tileInfo.HP > 20.0f)
                        result = new Vector2(8, 15);
                    else if (tileInfo.HP > 10.0f)
                        result = new Vector2(9, 15);
                    else if (tileInfo.HP >= 0.0f)
                        result = new Vector2(10, 15);



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
