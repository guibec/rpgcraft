using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample17_HelpDynamic : MonoBehaviour 
{
    // Similar to Inspect or Descriptor, the Help attribute can also be dynamic;
    [Inspect, Help("GetHelp")]
    public float myField;

    private HelpAttribute GetHelp()
    {
        if (myField > 0)
            return new HelpAttribute(HelpType.Info, "Change me to a negative value.");
        else if (myField < 0)
            return new HelpAttribute(HelpType.Warning, "Change me for positive!");
        else
            return new HelpAttribute(HelpType.Error, "Change me!!");
    }
}
