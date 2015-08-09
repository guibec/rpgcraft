using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample6_DescriptorDynamic : MonoBehaviour 
{
    // If you give only a single string to the descriptor attribute, it will try to find a function with that name.
    // Similar to the Inspect Dynamic example, it allows to change how something is drawn at runtime.
    [Inspect, Descriptor("GetDescriptor")]
    public string myName = "Change me!";

    private DescriptorAttribute GetDescriptor()
    {
        return new DescriptorAttribute(myName, "Every item can be changed");
    }
}
