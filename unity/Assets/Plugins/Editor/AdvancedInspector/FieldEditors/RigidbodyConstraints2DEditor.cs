using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class RigidbodyConstraints2DEditor : FieldEditor
    {
        private const int LABEL_WIDTH = 96;
        private const int TOGGLE_WIDTH = 28;

        public override bool EditDerived
        {
            get { return false; }
        }

        public override bool Expandable
        {
            get { return true; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(RigidbodyConstraints2D) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            field.Expandable = true;
            if (!field.Expanded)
                return;

            EditorGUI.showMixedValue = field.Mixed;

            EditorGUI.BeginChangeCheck();

            RigidbodyConstraints2D value = (RigidbodyConstraints2D)GetValue(field);
            int newValue = 0;

            EditorGUILayout.BeginHorizontal();
            GUILayout.Label("Freeze Position ", GUILayout.Width(LABEL_WIDTH));
            if (GUILayout.Toggle(value.Has(RigidbodyConstraints2D.FreezePositionX), "X", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints2D.FreezePositionX;

            if (GUILayout.Toggle(value.Has(RigidbodyConstraints2D.FreezePositionY), "Y", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints2D.FreezePositionY;
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.BeginHorizontal();
            GUILayout.Label("Freeze Rotation ", GUILayout.Width(LABEL_WIDTH));
            if (GUILayout.Toggle(value.Has(RigidbodyConstraints2D.FreezeRotation), "Z", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints2D.FreezeRotation;
            EditorGUILayout.EndHorizontal();

            if (EditorGUI.EndChangeCheck())
                field.SetValue(Enum.ToObject(typeof(RigidbodyConstraints2D), newValue));
        }
    }
}