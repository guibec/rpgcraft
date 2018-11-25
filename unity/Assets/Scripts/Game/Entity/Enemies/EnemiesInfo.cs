using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using SimpleJSON;

public static class EnemiesInfo
{
    public static string version { get; set; }

    public static Dictionary<string, EnemyInfo> m_enemiesInfo = new Dictionary<string, EnemyInfo>();

    static EnemiesInfo()
    {
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

    public static EnemyInfo GetInfoFromName(string name)
    {
        EnemyInfo enemyInfo = null;
        m_enemiesInfo.TryGetValue(name, out enemyInfo);
        return enemyInfo;
    }
}

public class EnemyInfo
{
    public int m_id { get; set; }
    public string m_name { get; set; }
    public List<Loot> m_loots { get; set; }

    public EnemyInfo(JSONNode node)
    {
        m_loots = new List<Loot>();

        m_id = node["id"].AsInt;
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
            for (int i = 0; i < item.count; ++i)
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

    public int count { get; set; }

    public Item(JSONNode itemNode)
    {
        string itemString = itemNode["item"];

        m_item = EItem.gel;
        EItem.TryParse(itemString, true, out m_item);

        if (itemNode["count"] == null)
        {
            count = 1;
        }
        else
        {
            count = itemNode["count"].AsInt;
        }
    }
}
