using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample23_Space : MonoBehaviour 
{
    // For readability, it is often useful to add empty space between items.
    // The Space attribute allows you to add empty space before or after an item.
    [Inspect, AdvancedInspector.Spacing(Before = 10)]
    public float myField;

    // Because Unity now has a UnityEngine.Space type, you have to declare it as AdvancedInspector.Space
    [Inspect, AdvancedInspector.Spacing(After = 10)]
    public float MyProperty
    {
        get { return myField; }
        set { myField = value; }
    }
}
