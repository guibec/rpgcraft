using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using SimpleJSON;

public class TileProperties
{
    public float SpeedFactor { get; private set; } = 1.0f;

    public TileProperties(JSONNode propertiesNode)
    {
        if (propertiesNode == null)
        {
            return;
        }

        if (propertiesNode["speedfactor"] != null)
        {
            SpeedFactor = propertiesNode["speedfactor"].AsFloat;
        }
    }
}
