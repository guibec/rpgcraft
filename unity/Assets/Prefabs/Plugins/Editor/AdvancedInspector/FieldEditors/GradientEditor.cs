using UnityEngine;
using UnityEditor;
using System;
using System.Collections;
using System.Reflection;

namespace AdvancedInspector
{
    public class GradientEditor : FieldEditor
    {
        private static MethodInfo gradientField;

        public GradientEditor()
        {
            // Because Unity does not expose this EditorGUI.
            gradientField = typeof(EditorGUILayout).GetMethod("GradientField", BindingFlags.NonPublic | BindingFlags.Static, null, new Type[] { typeof(Gradient), typeof(GUILayoutOption).MakeArrayType() }, null);
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Gradient) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            object value = GetValue(field);

            try
            {
                //Always throw "ExitGUIException"
                gradientField.Invoke(null, new object[] { value, null });
            }
            catch (Exception) { }
        }
    }
}