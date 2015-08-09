using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample21_Bypass : MonoBehaviour 
{
    // Some class - often in .dll - do now have AdvancedInspector attributes.
    // It is possible to force them to behave as if they were flagged with the Bypass attribute.
    // IMPORTANT: Some Unity classes are not meant to be inspected, and may break.
    // The Advanced Inspector should be rock solid in those case and flag invalid data field as read only.
    // The GUIStyle is one example of a class that has no editor for it.
    // The Bypass attribute automaticly inspect every public fields and properties.
    [Inspect, Bypass]
    public GUIStyle skin = new GUIStyle();
}
