using UnityEngine;
using System;
using System.Collections;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample13_Expandable : MonoBehaviour 
{
    // The expandable attribute is used to control if an item can not be expanded, or if it should be expanded by default.
    [Inspect, Expandable(Expanded = true)]
    public ExpandableClass expandedField;

    // Preventing an object from being expandable is sometime useful when you want to display its inner content differently.
    [Inspect, Expandable(Expandable = false)]
    public ExpandableClass notExpandableField;

    // This class is not expandable by default. 
    [Inspect]
    public NotExpandableClass notExpandableObject;

    // Class with the AdvancedInspector attribute are automaticly inlinable/expandable.
    [AdvancedInspector, Serializable]
    public class ExpandableClass
    {
        [Inspect]
        public float myField;
    }

    // It is possible to prevent that like this;
    // This is useful when you build your own base type similar to how a float is displayed.
    // A good example is the Guid class.
    [AdvancedInspector(Expandable = false), Serializable]
    public class NotExpandableClass
    {
        [Inspect]
        public float myField;
    }
}
