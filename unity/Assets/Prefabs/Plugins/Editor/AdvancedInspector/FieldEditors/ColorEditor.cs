using UnityEngine;
using UnityEditor;
using System;

namespace AdvancedInspector
{
    public class ColorEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Color), typeof(Color32) }; }
        }

        public override bool IsExpandable(InspectorField field)
        {
            return false;
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            object o = GetValue(field);
            Type type = o.GetType();

            Color color;
            if (type == typeof(Color32))
            {
                Color32 color32 = (Color32)o;
                color = color32;
            }
            else
                color = (Color)o;

            EditorGUI.BeginChangeCheck();
            color = EditorGUILayout.ColorField(color);

            if (EditorGUI.EndChangeCheck())
            {
                if (type == typeof(Color32))
                {
                    Color32 color32 = color;
                    field.SetValue(color32);
                }
                else
                    field.SetValue(color);
            }
        }
    }
}