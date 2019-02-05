﻿using System.Collections.Generic;
using Newtonsoft.Json;
using System;
using IronExtension;

namespace LootData
{
    public class Count
    {
        public int min;
        public int max;

        public Range<int> Range
        {
            get
            {
                return new Range<int>(min, max);
            }
        }

        public int Random()
        {
            if (min == max)
                return min;

            return Range.RandomValue();
        }
    }

    public class Item
    {
        public EItem name;
        public Count count;
    }

    public class Loot
    {
        public float probability;
        public List<Item> items;
    }
    public class LootInfo
    {
        public string name;
        public List<Loot> loots;

        public List<EItem> Random()
        {
            // First, add all probability
            float totalProbability = 0;
            foreach (var loot in loots)
            {
                totalProbability += loot.probability;
            }

            float dice = RandomManager.Next(0, totalProbability);

            // 0.3, 0.2, 0.4 --> 0.9
            // [0, 0.3] --> First Item
            // ]0.3, 0.5] --> Second Item
            // ]0.5, 0.9] --> Third Item
            Loot givenLoot = null;
            float cursor = 0;
            foreach (var loot in loots)
            {
                givenLoot = loot;
                cursor += loot.probability;
                if (cursor > dice)
                {
                    break;
                }
            }

            List<EItem> outputLoots = new List<EItem>();
            // Retrieve the items of the loot
            if (givenLoot == null)
            {
                return outputLoots;
            }

            // Not super optimal, but does the job
            foreach (var item in givenLoot.items)
            {
                int count = item.count.Range.RandomValue();

                for (int i = 0; i < count; ++i)
                {
                    outputLoots.Add(item.name);
                }
            }

            return outputLoots;
        }
    }

    public class LootsInfo
    {
        public string version;
        public List<LootInfo> lootsInfos;

        /// <summary>
        /// Built dynamically after deserializing
        /// </summary>
        [JsonIgnore]
        private Dictionary<string, LootInfo> m_cache = new Dictionary<string, LootInfo>();

        /// <summary>
        /// Load from file 
        /// </summary>
        /// <param name="filename"></param>
        public static LootsInfo CreateFromFile(string filename)
        {
            LootsInfo lootsInfo;

            try
            {
                lootsInfo = JSONUtils.LoadJSON<LootData.LootsInfo>(filename);
            }
            catch (Exception e)
            {
                UnityEngine.Debug.Log($"Failed to deserialize {filename} due to {e.Message}");
                return null;
            }

            if (lootsInfo == null)
            {
                UnityEngine.Debug.Log($"Failed to read {filename}");
                return null;
            }

            lootsInfo.UpdateCache();

            return lootsInfo;
        }

        /// <summary>
        /// Get LootInfo for the given enemy name
        /// </summary>
        /// <param name="name">Name of Enemy</param>
        /// <returns></returns>
        public LootInfo GetFromName(string name)
        {
            LootInfo lootInfo = null;
            m_cache.TryGetValue(name, out lootInfo);
            return lootInfo;
        }

        private void UpdateCache()
        {
            m_cache.Clear();

            if (lootsInfos == null)
                return;

            foreach (var keyValue in lootsInfos)
            {
                m_cache[keyValue.name] = keyValue;
            }
        }
    }
}

// TODO: This class is kinda useless now. Could just use LootData.LootsInfo directly
public class EnemiesInfo
{
    public LootData.LootsInfo m_lootsInfo;

    public LootData.LootsInfo Data
    {
        get
        {
            return m_lootsInfo;
        }
    }


    public EnemiesInfo()
    {
        string filename = "enemiesInfo";

        try
        {
            m_lootsInfo = LootData.LootsInfo.CreateFromFile(filename);
        }
        catch (Exception e)
        {
            UnityEngine.Debug.Log($"Failed to deserialize {filename} due to {e.Message}");
            return;
        }

        if (m_lootsInfo == null)
        {
            UnityEngine.Debug.Log($"Failed to read {filename}");
            return;
        }
    }
}
