using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class CharEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(char) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            GUILayout.BeginHorizontal();

            object value = GetValue(field);
            string text = "";
            if (value != null)
                text = value.ToString();

            EditorGUI.BeginChangeCheck();

            string result = EditorGUILayout.TextField(text);
            
            char c;
            if (result.Length == 0)
                c = '\0';
            else
                c = result[0];

            if (EditorGUI.EndChangeCheck())
                field.SetValue(c);

            GUILayout.EndHorizontal();
        }
    }
}