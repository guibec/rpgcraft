using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class RigidbodyConstraintsEditor : FieldEditor
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
            get { return new Type[] { typeof(RigidbodyConstraints) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            field.Expandable = true;
            if (!field.Expanded)
                return;

            EditorGUI.showMixedValue = field.Mixed;

            EditorGUI.BeginChangeCheck();

            RigidbodyConstraints value = (RigidbodyConstraints)GetValue(field);
            int newValue = 0;

            EditorGUILayout.BeginHorizontal();
            GUILayout.Label("Freeze Position ", GUILayout.Width(LABEL_WIDTH));
            if (GUILayout.Toggle(value.Has(RigidbodyConstraints.FreezePositionX), "X", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints.FreezePositionX;

            if (GUILayout.Toggle(value.Has(RigidbodyConstraints.FreezePositionY), "Y", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints.FreezePositionY;

            if (GUILayout.Toggle(value.Has(RigidbodyConstraints.FreezePositionZ), "Z", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints.FreezePositionZ;
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.BeginHorizontal();
            GUILayout.Label("Freeze Rotation ", GUILayout.Width(LABEL_WIDTH));
            if (GUILayout.Toggle(value.Has(RigidbodyConstraints.FreezeRotationX), "X", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints.FreezeRotationX;

            if (GUILayout.Toggle(value.Has(RigidbodyConstraints.FreezeRotationY), "Y", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints.FreezeRotationY;

            if (GUILayout.Toggle(value.Has(RigidbodyConstraints.FreezeRotationZ), "Z", GUILayout.Width(TOGGLE_WIDTH)))
                newValue += (int)RigidbodyConstraints.FreezeRotationZ;
            EditorGUILayout.EndHorizontal();

            if (EditorGUI.EndChangeCheck())
                field.SetValue(Enum.ToObject(typeof(RigidbodyConstraints), newValue));
        }
    }
}