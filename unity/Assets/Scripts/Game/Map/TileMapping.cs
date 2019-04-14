using UnityEngine;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using TileData;

namespace TileData
{
    public class TileResourceDef
    {
        // JSON properties
        public string Filename { set; get; }

        // Pixel Rect (ex: going from 0 to 1023)
        public Rect PixelRect { set; get; }

        public int Count { set; get; } = 1;

        // DYNAMIC Properties
        // UVs Rect, computed dynamically (going from 0.f to 1.f)
        public Rect Rect { set; get; }

        /// <summary>
        /// When a tile can refer to multiple phases, the PixelRect are also stored here.
        /// </summary>
        public List<Rect> PixelRectList { get; set; }

        /// <summary>
        /// When a tile can refer to multiple phases, the other Rect are stored here
        /// </summary>
        public List<Rect> RectList { set; get; }

        [OnDeserialized]
        internal void OnDeserialized(StreamingContext context)
        {
            // Build the PixelRectList from 
            PixelRectList = new List<Rect>(Count);

            // We also need to prebuild the RectList
            RectList = new List<Rect>(Count);

            PixelRectList.Add(PixelRect);
            RectList.Add(Rect);

            Rect slidingRect = PixelRect;
            for (int i = 1; i < Count; i++)
            {
                float width = slidingRect.width;
                slidingRect.xMin += width + 2;
                slidingRect.xMax += width + 2;
                PixelRectList.Add(slidingRect);

                // Add a dummy Rect for now, it will be fixed in the atlas mapping
                RectList.Add(Rect);
            }


            
        }
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

    public Texture2D AtlasTexture
    {
        get { return m_atlasTexture; }
    }

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
            string filename = tilesInfo.Value.Resource.Filename;
            if (filename != "")
            {
                filesSet.Add(filename);
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

        // we will build this map as we go along
        Dictionary<string, Texture2D> mapPathToTexture = new Dictionary<string, Texture2D>(files.Length);
        Dictionary<string, int> mapPathToTextureIndex = new Dictionary<string, int>(files.Length);

        HashSet<string> loadedFiles = new HashSet<string>();

        int texIndex = 0;
        foreach (var file in files)
        {
            foreach (var tileInfo in m_tilesInfo.tilesInfo)
            {
                string texturePath = tileInfo.Value.Resource.Filename;
                if (texturePath == file)
                {
                    // Only load the texture if it has not been loaded so far
                    if (!loadedFiles.Contains(texturePath))
                    {
                        var texture = Resources.Load<Texture2D>(texturePath);
                        if (texture != null)
                        {
                            loadedFiles.Add(texturePath);
                            mapPathToTexture.Add(texturePath, texture);
                            mapPathToTextureIndex.Add(texturePath, texIndex);

                            textures[texIndex] = texture;
                            texIndex++;
                        }
                        else
                        {
                            Debug.LogError($"Could not load Resource {texturePath}");
                            break;
                            // TODO: Handle can't load texture
                        }
                    }
                }
            }
        }

        // At this point, all the textures have been loaded into the Texture2D[] field
        // Create the atlas
        const int atlasSize = 8192;
        string allPathsCSV = string.Join(", ", mapPathToTextureIndex.Keys.ToList());
        Debug.Log($"Going to pack {mapPathToTextureIndex.Count} textures: {allPathsCSV}");
        Rect[] rects = m_atlasTexture.PackTextures(textures, 2, atlasSize, true);

        if (rects == null)
        {
            Debug.LogError("Could not create texture atlas");
            return;
        }

        // All rects are based from 0 to 1
        var keys = mapPathToTextureIndex.Keys.ToList();
        for (int index = 0; index != keys.Count; index++)
        {
            Debug.Log($"Texture {keys[index]} now at Rect {rects[index]}");
        }

        int newTextureWidth = m_atlasTexture.width;
        int newTextureHeight = m_atlasTexture.height;

        // The atlas has been created. It's time to update the Rect information for each resource
        foreach (var tileInfo in m_tilesInfo.tilesInfo)
        {
            string texturePath = tileInfo.Value.Resource.Filename;

            if (texturePath == "")
                continue;

            bool foundTexture = mapPathToTexture.TryGetValue(texturePath, out Texture2D texture);
            bool foundIndex = mapPathToTextureIndex.TryGetValue(texturePath, out int textureIndex);

            if (!foundTexture || !foundIndex)
            {
                Debug.LogError($"Could not remap texture atlas for {texturePath}");
                continue;
            }

            int originalTextureWidth = texture.width;
            int originalTextureHeight = texture.height;

            // Remap to the new UVs
            Rect destTextureRect = rects[textureIndex];

            for (int i = 0; i < tileInfo.Value.Resource.Count; i++)
            {
                Rect originalRect = tileInfo.Value.Resource.PixelRectList[i];

                // Original Rect Coordinates are in :
                // Pixels, going from bottom left to upper right.
                // In absolute coordinates (ex: 0 ... 512)
                // Once converted, they will be in UV, going from bottom-left to upper right
                // In relative coordinates (0.0f ... 1.0f)
                Rect newRect = new Rect();
                newRect.xMin = destTextureRect.xMin + originalRect.xMin / originalTextureWidth * destTextureRect.width;
                newRect.yMin = destTextureRect.yMin + originalRect.yMin / originalTextureHeight * destTextureRect.height;
                newRect.width = originalRect.width / originalTextureWidth * destTextureRect.width;
                newRect.height = originalRect.height / originalTextureHeight * destTextureRect.height;

                // Need to update the PixelRect too !
                Rect newPixelRect = new Rect();
                newPixelRect.xMin = newRect.xMin * newTextureWidth;
                newPixelRect.yMin = newRect.yMin * newTextureHeight;
                newPixelRect.width = newRect.width * newTextureWidth;
                newPixelRect.height = newRect.height * newTextureHeight;

                Debug.Log($"Remapping {tileInfo.Key} of {texturePath} from {tileInfo.Value.Resource.RectList[i]} to atlas at {newRect}");
                Debug.Log(
                    $"Remapping {tileInfo.Key} of {texturePath} from pixel {tileInfo.Value.Resource.PixelRectList[i]} to {newPixelRect}");

                if (i == 0)
                {
                    tileInfo.Value.Resource.Rect = newRect;
                    tileInfo.Value.Resource.PixelRect = newPixelRect;
                }

                tileInfo.Value.Resource.RectList[i] = newRect;
                tileInfo.Value.Resource.PixelRectList[i] = newPixelRect;
            }
        }
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
            int tileOffset = 0;
            if (tileDef.Resource.Count > 1)
            {
                if (tileInfo.HP == 100f)
                    tileOffset = 0;
                else if (tileInfo.HP >= 90f)
                    tileOffset = 1;
                else if (tileInfo.HP >= 80f)
                    tileOffset = 2;
                else if (tileInfo.HP >= 70f)
                    tileOffset = 3;
                else if (tileInfo.HP >= 60f)
                    tileOffset = 4;
                else if (tileInfo.HP >= 50f)
                    tileOffset = 5;
                else if (tileInfo.HP >= 40f)
                    tileOffset = 6;
                else if (tileInfo.HP >= 30f)
                    tileOffset = 7;
                else if (tileInfo.HP >= 20f)
                    tileOffset = 8;
                else if (tileInfo.HP >= 10f)
                    tileOffset = 9;
                else if (tileInfo.HP >= 0f)
                    tileOffset = 10;
            }


            if (tileOffset < 0 || tileOffset >= tileDef.Resource.RectList.Count)
            {
                int err;
                err = 0;
            }

            Rect uvRect = tileDef.Resource.RectList[tileOffset];
            ul = new Vector2(uvRect.xMin, uvRect.yMin);
            ur = new Vector2(uvRect.xMax, uvRect.yMin);
            bl = new Vector2(uvRect.xMin, uvRect.yMax);
            br = new Vector2(uvRect.xMax, uvRect.yMax);
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
