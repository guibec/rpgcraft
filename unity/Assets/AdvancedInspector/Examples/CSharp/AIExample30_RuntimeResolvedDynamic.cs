using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample30_RuntimeResolvedDynamic : MonoBehaviour 
{
    [Inspect]
    public Material mat;

    [SerializeField]
    private UnityEngine.Object myField;

    // Using the RuntimeResolve with a linked function allows you to restrist the type of object that shows up in Unity's object picker.
    // It is sometimes useful when you want a dynamic property to only show a specific type of object at a specific moment.
    // In this example, only Material will show up as being valid, even if MyProperty is null.
    [Inspect, RuntimeResolve("GetResolvedType")]
    public object MyProperty
    {
        get { return myField; }
        set { myField = value as UnityEngine.Object; }
    }

    private Type GetResolvedType()
    {
        return typeof(Material);
    }
}
