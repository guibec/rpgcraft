using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample14_Background : MonoBehaviour 
{
    // The background attribute is simply used to stored a color for the background of an expandable item.
    [Inspect, Background(1, 0.5f, 0)]
    public ExpandableClass myObject;

    [AdvancedInspector, Serializable]
    public class ExpandableClass
    {
        [Inspect]
        public float myField;
    }

    // It can also be dynamic.
    [Inspect, Background("GetBackgroundColor")]
    public float[] myArray;

    [Inspect]
    public Color color;

    private Color GetBackgroundColor()
    {
        return color;
    }
}
