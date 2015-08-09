using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample5_Descriptor : MonoBehaviour 
{
    // The Descriptor attribute is used to give more information about an item.
    // It can also change the displayed name of the item;
    [Inspect, Descriptor("Variable New Name", "")]
    public float myFloat;

    // The second parameter - "Description" - is used as a tooltip in the inspector.
    [Inspect, Descriptor("With Tooltip", "This is the tooltip description")]
    public float myFloat2;

    // The third parameter - "URL" - is used as a web info in the inspector.
    // Try right-clicking on this label to see "Online Help". 
    [Inspect, Descriptor("With URL", "This is the tooltip description", "http://www.lightstrikersoftware.com/")]
    public float myFloat3;

    // The descriptor also contains a RGB value, that is used to tint the item.
    // Because of the limits of attributes, the color has to be passed by floats.
    [Inspect, Descriptor("With Color", "This is the tooltip description", "", 1, 0, 0)]
    public float myFloat4;
}
