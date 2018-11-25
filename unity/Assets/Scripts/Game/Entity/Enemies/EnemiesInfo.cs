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
        string filename = "enemiesInfo.json";

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
            m_enemiesInfo[enemyInfo.name] = enemyInfo;
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
    public int id { get; set; }
    public string name { get; set; }
    public List<Loot> loots { get; set; }

    public EnemyInfo(JSONNode node)
    {
        loots = new List<Loot>();

        id = node["id"].AsInt;
        name = node["name"].Value;

        JSONNode lootNodeRoot = node["loot"];

        foreach (JSONNode lootNode in lootNodeRoot.Childs)
        {
            loots.Add(new Loot(lootNode));
        }
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

        probability = node["probability"].AsInt;
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
    public string item { get; set; }
    public int count { get; set; }

    public Item(JSONNode itemNode)
    {
        item = itemNode["item"]; // TODO: Get the actual Item reference here

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
