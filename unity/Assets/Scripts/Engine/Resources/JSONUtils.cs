using UnityEngine;
using Newtonsoft.Json;

public class JSONUtils { 
    public static T LoadJSON<T>(string filename)
    {
        TextAsset textAsset = Resources.Load(filename) as TextAsset;
        if (textAsset == null) return default;

        Newtonsoft.Json.Serialization.ITraceWriter traceWriter = new Newtonsoft.Json.Serialization.MemoryTraceWriter
            { LevelFilter = System.Diagnostics.TraceLevel.Verbose };

        var resolver = new Newtonsoft.Json.Serialization.CamelCasePropertyNamesContractResolver();

        var settings = new JsonSerializerSettings
            { TraceWriter = traceWriter, ContractResolver = resolver };

        T obj = JsonConvert.DeserializeObject<T>(textAsset.text, settings);

        Debug.Log($"Deserialize trace output {traceWriter}");

        return obj;

    }

}
