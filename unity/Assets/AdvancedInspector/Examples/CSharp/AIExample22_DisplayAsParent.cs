using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample22_DisplayAsParent : MonoBehaviour 
{
    // The DisplayAsParent attribute prevents a sub-object from being expandable
    // Instead it display its inner fields as being part of the parent.
    // This is useful to remove some depth in multi-nested-object.
    // The name of this field remains, but it shows no editor.
    [Inspect, DisplayAsParent]
    public SubObject myObject = new SubObject();

    [AdvancedInspector]
    public class SubObject
    {
        [Inspect]
        public float myField;
    }
}
