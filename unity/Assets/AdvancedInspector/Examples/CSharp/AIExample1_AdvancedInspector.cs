using UnityEngine;

// Adding the AdvancedInspector namespace is required when using AI's features like the attributes.
using AdvancedInspector;

// The AdvancedInspector attribute is a switch between Unity inspector and the Advanced Inspector.
// By default, Advanced Inspector tries to draw everything it can even without that attribute.
// This behaviour can be switched off in the preference panel.
// All the example have that attribute so they work even if you turn that default inspection behaviour.
// However, you are not required to add it to your class.

namespace AdvancedInspector
{
    // The first optionnal parameter allows the Advanced Inspector to work in the same manner as the default one.
    [AdvancedInspector(true)] // [AdvancedInspector] or [AdvancedInspector(false)]
    public class AIExample1_AdvancedInspector : MonoBehaviour
    {
        // Usually Advanced Inspector would not show this item because it lacks the "Inspect" attribute.
        public float myFloat;
    }
}

// The AdvancedInspector attribute contains 3 property;
// InspectDefaultItems; it displayes fields Unity would display by default.
// ShowScript; show or hide the top "script" field.
// Expandable; In case of an inlined variable, it can force the item to not be expandable (have an arrow on its left)