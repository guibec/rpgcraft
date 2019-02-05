#define DEBUG
#define TRACE

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
            Newtonsoft.Json.Serialization.ITraceWriter traceWriter = new Newtonsoft.Json.Serialization.MemoryTraceWriter
                { LevelFilter = System.Diagnostics.TraceLevel.Verbose };

            JsonSerializerSettings settings = new JsonSerializerSettings
                { TraceWriter = traceWriter };

            T obj = JsonConvert.DeserializeObject<T>(textAsset.text, settings);

            UnityEngine.Debug.Log($"Deserialize trace output {traceWriter.ToString()}");

            return obj;
        }

        return default(T);
    }

}
