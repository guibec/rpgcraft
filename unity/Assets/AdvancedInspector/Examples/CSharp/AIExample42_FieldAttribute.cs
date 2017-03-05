using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Unlike PropertyDrawers, FieldDrawers can be applied to property, method or non-serialized field.
    /// </summary>
    public class AIExample42_FieldAttribute : MonoBehaviour
    {
        [Inspect, FieldAttributeExample]
        private string Text
        {
            get { return "This is some non-serialized property text"; }
        }

        [Inspect, FieldAttributeExample]
        private string[] Texts
        {
            get { return new string[] { "To have an attribute pass down to", "Collection elements, it must", "Implement the IListAttribute interface." }; }
        }
    }
}
