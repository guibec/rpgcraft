using System.IO;
using UnityEngine;
using SimpleJSON;
using Newtonsoft.Json;

public class JSONUtils { 

    public static JSONNode ParseJSON(string filename)
    {
        TextAsset textAsset = Resources.Load(filename) as TextAsset;
        if (textAsset != null)
        {
            return JSON.Parse(textAsset.text);
        }

        return null;
    }

    public static T LoadJSON<T>(string filename)
    {
        TextAsset textAsset = Resources.Load(filename) as TextAsset;
        if (textAsset != null)
        {
            return JsonConvert.DeserializeObject<T>(textAsset.text);
        }

        return default(T);
    }

}
