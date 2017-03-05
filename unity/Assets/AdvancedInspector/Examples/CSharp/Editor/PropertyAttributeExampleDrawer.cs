using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CustomPropertyDrawer(typeof(PropertyAttributeExample))]
    public class PropertyAttributeExampleDrawer : PropertyDrawer
    {
        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            EditorGUI.LabelField(position, "This is a attribute-bound property drawers.");
        }
    }
}