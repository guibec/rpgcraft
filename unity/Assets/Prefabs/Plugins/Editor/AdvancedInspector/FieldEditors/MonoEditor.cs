using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace AdvancedInspector
{
    public class MonoEditor : FieldEditor
    {
        public override bool EditDerived
        {
            get { return false; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(MonoScript) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            if (field.SerializedProperty == null || field.SerializedProperty.serializedObject == null)
                return;

            Type type = field.SerializedInstances[0].GetType();
            if (typeof(ComponentMonoBehaviour).IsAssignableFrom(type))
            {
                if (GUILayout.Button(type.Name, EditorStyles.label))
                {
                    EditorGUIUtility.PingObject(MonoScript.FromMonoBehaviour(field.SerializedInstances[0] as MonoBehaviour));
                }
            }
            else
            {
                EditorGUI.BeginChangeCheck();

                field.SerializedProperty.serializedObject.Update();
                EditorGUILayout.PropertyField(field.SerializedProperty, new GUIContent(""));
                field.SerializedProperty.serializedObject.ApplyModifiedProperties();

                if (EditorGUI.EndChangeCheck())
                {
                    if (field.Parent != null)
                        field.Parent.RefreshFields();
                    else
                        AdvancedInspectorControl.Editor.Instances = new object[] { field.SerializedProperty.serializedObject.targetObject };
                }
            }

            DrawObjectSelector(field);
        }

        private void DrawObjectSelector(InspectorField field)
        {
            MonoBehaviour behaviour = field.GetValue() as MonoBehaviour;
            if (behaviour == null)
                return;

            List<Component> components = new List<Component>(behaviour.gameObject.GetComponents(field.BaseType));
            if (components.Count == 1)
                return;

            int index = components.IndexOf(behaviour);
            string[] texts = new string[components.Count];

            for (int i = 0; i < components.Count; i++)
                texts[i] = i.ToString() + " : " + components[i].ToString();

            EditorGUILayout.BeginHorizontal();
            int selection = EditorGUILayout.Popup(index, texts);
            EditorGUILayout.EndHorizontal();

            if (selection == index)
                return;

            field.SetValue(components[selection]);
        }
    }
}