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
        public string Name { get; set; } = "";
        public TileResourceDef Resource { get; set; }

        public TileProperties Properties { get; set; } = new TileProperties();
    }

    public class TilesInfo
    {
        public string version;
        public Dictionary<ETile, TileDef> tilesInfo = new Dictionary<ETile, TileDef>(50);
        public Dictionary<ETile, ETile> mining = new Dictionary<ETile, ETile>(50);
    }
}

public class TileMapping : MonoSingleton<TileMapping>
{
    private TilesInfo m_tilesInfo;
    private Texture2D m_atlasTexture;

    public TileResourceDef GetTileResourceDef(ETile tile)
    {
        return !m_tilesInfo.tilesInfo.TryGetValue(tile, out var tileDef) ? null : tileDef.Resource;
    }

    public TileProperties GetTileProperties(ETile tile)
    {
        return !m_tilesInfo.tilesInfo.TryGetValue(tile, out var tileDef) ? new TileProperties() : tileDef.Properties;
    }

    /// <summary>
    /// Load the file information from the json file specified in filename. 
    /// </summary>
    /// <param name="filename">The json file found in the Resource asset</param>
    /// <returns>True if the file was properly loaded</returns>
    public bool BuildFromJSON(string filename)
    {
        m_tilesInfo = JSONUtils.LoadJSON<TilesInfo>(filename);

        if (m_tilesInfo == null)
        {
            return false;
        }

        UpdateAtlas();

        return true;
    }

    private string[] GetUniqueFilenames()
    {
        HashSet<string> filesSet = new HashSet<string>();

        foreach (var tilesInfo in m_tilesInfo.tilesInfo)
        {
            if (!tilesInfo.Value.Resource.Filename.Empty)
            {
                filesSet.Add(tilesInfo.Value.Resource.Filename);
            }
        }

        string[] ret = new string[filesSet.Count];
        filesSet.CopyTo(ret);
        return ret;
    }

    private void UpdateAtlas()
    {
        m_atlasTexture = new Texture2D(8192, 8192);

        // first pass loop checking how many unique filenames we have
        string[] files = GetUniqueFilenames();
        Texture2D[] textures = new Texture2D[files.Length];

        int texIndex = 0;
        foreach (var file in files)
        {
            foreach (var tileInfo in m_tilesInfo.tilesInfo)
            {
                string texturePath = tileInfo.Value.Resource.Filename;
                if (texturePath == file)
                {
                    var texture = Resources.Load<Texture2D>(texturePath);
                    if (texture != null)
                    {
                        textures[texIndex] = texture;
                        texIndex++;
                    }
                    else
                    {
                        // TODO: Handle can't load texture
                    }

                    break;
                }
            }
        }
        
        // At this point, all the textures have been loaded into the Texture2D[] field
        // Create the atlas

        Rect[] rects = m_atlasTexture.PackTextures(textures, 2, 8192, true);


    }

    /// <summary>
    /// Return the new ETile from input after it has been mined. If input cannot be mined, return Invalid
    /// </summary>
    /// <param name="input">The tile to mine</param>
    /// <returns></returns>
    public ETile GetMiningTransform(ETile input)
    {
        return m_tilesInfo.mining.TryGetValue(input, out ETile result) ? result : ETile.Invalid;
    }
    
    public void GetUVFromTile(TileInfo tileInfo, out Vector2 ul, out Vector2 ur, out Vector2 bl, out Vector2 br)
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
