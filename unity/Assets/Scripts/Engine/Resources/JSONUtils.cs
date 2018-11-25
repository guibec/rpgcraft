using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using SimpleJSON;

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
}
