using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class BooleanEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(bool) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            object value = GetValue(field);

            EditorGUI.BeginChangeCheck();
            bool result;
            if (style != null)
                result = EditorGUILayout.Toggle((bool)value, style);
            else
                result = EditorGUILayout.Toggle((bool)value);

            if (EditorGUI.EndChangeCheck())
                field.SetValue(result);
        }
    }
}