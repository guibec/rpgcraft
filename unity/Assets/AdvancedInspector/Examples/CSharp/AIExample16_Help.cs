using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample16_Help : MonoBehaviour 
{
    // "Help Box" is a bubble of information added before or after the inspected item.
    [Inspect, Help(HelpType.Info, "This is an help box")]
    public float myFirstField;

    // It is possible to control where the box appear, the default being after;
    [Inspect, Help(HelpType.Warning, HelpPosition.Before, "This box appear before the item.")]
    public float mySecondField;
}
