using UnityEngine;
using UnityEngine.Events;

namespace AdvancedInspector
{
    public class AIExample37_UnityEvent : MonoBehaviour
    {
        // Advanced Inspector is able to draw any already existing Property Drawer.
        // For example, Unity Event here are drawn from Unity's own drawer.
        // However, drawer are bound to serialization, and therefor cannot be used in a property.
        public UnityEvent testEvent;
    }
}