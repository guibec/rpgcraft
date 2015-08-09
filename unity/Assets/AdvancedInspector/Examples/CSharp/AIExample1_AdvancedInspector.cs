using UnityEngine;
using System.Collections;
using System.Collections.Generic;

// Adding the AdvancedInspector namespace is required.
using AdvancedInspector;

// The AdvancedInspector attribute is the switch between Unity inspector and the Advanced Inspector.
// Without it, this class is drawn by the default inspector.
// The first optionnal parameter allows the Advanced Inspector to work in the same manner as the default one.
[AdvancedInspector(true)] // [AdvancedInspector] or [AdvancedInspector(false)]
public class AIExample1_AdvancedInspector : MonoBehaviour
{
    // Usually Advanced Inspector would not show this item because it lacks the "Inspect" attribute.
    public float myFloat;
}

// The AdvancedInspector attribute contains 3 property;
// InspectDefaultItems; it displayes fields Unity would display by default.
// ShowScript; show or hide the top "script" field.
// Expandable; In case of an inlined variable, it can force the item to not be expandable (have an arrow on its left)