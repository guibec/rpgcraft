using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using SimpleJSON;
using Newtonsoft.Json;
using System.IO;
using IronExtension;

namespace LootData
{
    public class Count
    {
        public int min;
        public int max;
    }

    public class Item
    {
        public string name;
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
    }

    public class LootsInfo
    {
        public string version;
        public List<LootInfo> lootsinfo;
    }
}

public class EnemiesInfo
{
    public string version { get; set; }

    public Dictionary<string, EnemyInfo> m_enemiesInfo = new Dictionary<string, EnemyInfo>();

    public EnemiesInfo()
    {
        // Simple test to see if the data structure holds up
        LootData.Count count = new LootData.Count();
        count.min = 2;
        count.max = 4;

        LootData.Item item = new LootData.Item();
        item.name = "stone";
        item.count = count;

        LootData.Loot loot = new LootData.Loot();
        loot.probability = 0.5f;
        loot.items = new List<LootData.Item>();
        loot.items.Add(item);

        LootData.LootInfo lootInfo = new LootData.LootInfo();
        lootInfo.name = "Slime";
        lootInfo.loots = new List<LootData.Loot>();
        lootInfo.loots.Add(loot);

        LootData.LootsInfo lootsInfo = new LootData.LootsInfo();
        lootsInfo.version = "1.0";
        lootsInfo.lootsinfo = new List<LootData.LootInfo>();
        lootsInfo.lootsinfo.Add(lootInfo);

        string serializedData = JsonConvert.SerializeObject(lootsInfo, Formatting.Indented);

        StreamWriter writer = new StreamWriter(Application.persistentDataPath + "test.json", false);
        writer.Write(serializedData);
        writer.Close();
        Debug.Log(string.Format("Wrote to {0}", Application.persistentDataPath + "test.json"));

        string filename = "enemiesInfo";

        JSONNode rootNode = JSONUtils.ParseJSON(filename);

        if (rootNode == null)
        {
            return;
        }

        version = rootNode["version"].Value;
        JSONNode enemiesInfoJSON = rootNode["enemiesInfo"];

        foreach (JSONNode node in enemiesInfoJSON.Childs)
        { 
            EnemyInfo enemyInfo = new EnemyInfo(node);
            m_enemiesInfo[enemyInfo.m_name] = enemyInfo;
        }
    }

    public EnemyInfo GetInfoFromName(string name)
    {
        EnemyInfo enemyInfo = null;
        m_enemiesInfo.TryGetValue(name, out enemyInfo);
        return enemyInfo;
    }
}

public class EnemyInfo
{
    public string m_name { get; set; }
    public List<Loot> m_loots { get; set; }

    public EnemyInfo(JSONNode node)
    {
        m_loots = new List<Loot>();

        m_name = node["name"].Value;

        JSONNode lootNodeRoot = node["loot"];

        foreach (JSONNode lootNode in lootNodeRoot.Childs)
        {
            m_loots.Add(new Loot(lootNode));
        }
    }

    public List<EItem> RandomLoot()
    {
        // First, add all probability
        List<EItem> outputLoots = new List<EItem>();

        float totalProbability = 0;
        foreach (var loot in m_loots)
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
        foreach (var loot in m_loots)
        {
            givenLoot = loot;
            cursor += loot.probability;
            if (cursor > dice)
            {
                break;
            }
        }

        // Retrieve the items of the loot
        if (givenLoot == null)
        {
            return outputLoots;
        }

        // Not super optimal, but does the job
        foreach (var item in givenLoot.item)
        {
            int count = item.m_range.RandomValue();

            for (int i = 0; i < count; ++i)
            {
                for (int j = 0; j < givenLoot.count; j++)
                {
                    outputLoots.Add(item.m_item);
                }
            }
        }

        return outputLoots;

    }
}

public class Loot
{
    public float probability { get; set; }

    public int count { get; set; }

    public List<Item> item { get; set; }

    public Loot(JSONNode node)
    {
        item = new List<Item>();

        probability = node["probability"].AsFloat;
        if (node["count"] == null)
        {
            count = 1;
        }
        else
        {
            count = node["count"].AsInt;
        }

        // item is either a single item, or a list of items
        if (node["item"] == null)
        {
            return;
        }

        if (node["item"].Count == 0)
        {
            count = 1;
            item.Add(new Item(node));
        }
        else
        {
            foreach (JSONNode itemNode in node["item"].Childs)
            {
                item.Add(new Item(itemNode));
            }
        }
    }
}

public class Item
{
    public EItem m_item;

    public Range<int> m_range = new Range<int>();

    public Item(JSONNode itemNode)
    {
        string itemString = itemNode["item"];

        m_item = EItem.Gel;
        EItem.TryParse(itemString, true, out m_item);

        if (itemNode["count"].AsObject != null)
        {
            JSONNode countNode = itemNode["count"].AsObject;
            if (countNode["min"] != null && countNode["max"] != null)
            {
                m_range.Minimum = countNode["min"].AsInt;
                m_range.Maximum = countNode["max"].AsInt;
            }
            else
            {
                m_range.Minimum = m_range.Maximum = 1;
            }
        }
        else if (itemNode["count"] != null)
        {
            m_range.Minimum = m_range.Maximum = itemNode["count"].AsInt;
        }
        else
        {
            m_range.Minimum = m_range.Maximum = 1;
        }
    }
}
