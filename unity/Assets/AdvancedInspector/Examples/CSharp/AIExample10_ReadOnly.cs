using UnityEngine;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample10_ReadOnly : MonoBehaviour 
{
    // There's a few ways to display an item but prevent it from being edited.
    // The first is using the ReadOnly attribute;
    [Inspect, ReadOnly]
    public float myField;

    // A property with only a getter is also not editable.
    [Inspect]
    public float MyGetter
    {
        get { return myField; }
    }

    // Everything can be turned "Read Only", even method.
    [Inspect, ReadOnly]
    public void MyMethod()
    {
        myField++;
    }
}
