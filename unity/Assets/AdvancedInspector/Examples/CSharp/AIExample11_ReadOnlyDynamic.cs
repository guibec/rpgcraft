using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample11_ReadOnlyDynamic : MonoBehaviour 
{
    // The ReadOnly attribute can also be dynamic and change at run time.
    [Inspect, ReadOnly("IsReadOnly")]
    public bool myVariable;

    // The function can be private or public, it doesn't matter.
    private bool IsReadOnly()
    {
        return isReadOnly;
    }

    private bool isReadOnly = true;

    // In this example, the button toggle on/off the read only attribute of "myVariable".
    [Inspect]
    public void PressMe()
    {
        isReadOnly = !isReadOnly;
    }
}
