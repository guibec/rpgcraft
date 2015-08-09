using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample39_DynamicStatic : MonoBehaviour
{
    // A Runtime Dynamic attribute can also recieve the path towards a static method declared in any type.
    // This offers a way to write generic conditionals.
    // The '.' denote a path towards a method. The class owning the method can be nested.
    [Inspect("StaticDynamicExample.Inspect")]
    public bool myVariable;

    public bool displayItem = true;

    // In this example, the button toggle on/off the display of "myVariable".
    [Inspect]
    public void PressMe()
    {
        displayItem = !displayItem;
    }

    [Inspect, Help(HelpAttribute.IsNull, HelpType.Error, "Should not be null!")]
    public Camera nullField;
}

public class StaticDynamicExample
{
    // The delegate should have a single param argument.
    // Usually, Advanced Inspector pass the instance and the value as argument.
    // Other attribute may pass extra argument, such as Regex or the attribute properties. 
    private static bool Inspect(InspectAttribute inspect, object instance, object value)
    {
        AIExample39_DynamicStatic example = instance as AIExample39_DynamicStatic;
        if (example == null)
            return false;

        return example.displayItem;
    }
}