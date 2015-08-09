using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class VectorEditor : FieldEditor
    {
        public override bool Expandable
        {
            get { return false; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Vector2), typeof(Vector3), typeof(Vector4) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            Type type = field.BaseType;

            float width = EditorGUIUtility.labelWidth;
            EditorGUIUtility.labelWidth = VECTOR_FIELD_WIDTH;

            GUILayout.BeginHorizontal();
            if (type == typeof(Vector2))
            {
                Vector2[] values = field.GetValues<Vector2>();

                float[] x = new float[values.Length];
                float[] y = new float[values.Length];

                for (int i = 0; i < values.Length; i++)
                {
                    x[i] = values[i].x;
                    y[i] = values[i].y;
                }

                float result;
                if (FloatEditor.DrawFloat("X", x, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " X");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].x = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("Y", y, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Y");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].y = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }
            }
            else if (type == typeof(Vector3))
            {
                Vector3[] values = field.GetValues<Vector3>();

                float[] x = new float[values.Length];
                float[] y = new float[values.Length];
                float[] z = new float[values.Length];

                for (int i = 0; i < values.Length; i++)
                {
                    x[i] = values[i].x;
                    y[i] = values[i].y;
                    z[i] = values[i].z;
                }

                float result;
                if (FloatEditor.DrawFloat("X", x, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " X");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].x = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("Y", y, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Y");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].y = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("Z", z, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Z");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].z = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }
            }
            else if (type == typeof(Vector4))
            {
                Vector4[] values = field.GetValues<Vector4>();

                float[] x = new float[values.Length];
                float[] y = new float[values.Length];
                float[] z = new float[values.Length];
                float[] w = new float[values.Length];

                for (int i = 0; i < values.Length; i++)
                {
                    x[i] = values[i].x;
                    y[i] = values[i].y;
                    z[i] = values[i].z;
                    w[i] = values[i].w;
                }

                float result;
                if (FloatEditor.DrawFloat("X", x, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " X");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].x = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("Y", y, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Y");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].y = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("Z", z, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Z");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].z = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("W", w, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " W");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].w = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }
            }
            GUILayout.EndHorizontal();

            EditorGUIUtility.labelWidth = width;
        }

        public override void OnContextualClick(InspectorField field, GenericMenu menu)
        {
            menu.AddItem(new GUIContent("Zero"), false, Zero);
            menu.AddItem(new GUIContent("One"), false, One);

            menu.AddSeparator("");
        }

        private void Zero()
        {
            if (AdvancedInspectorControl.Field.Type == typeof(Vector2))
                AdvancedInspectorControl.Field.SetValue(Vector2.zero);
            else if (AdvancedInspectorControl.Field.Type == typeof(Vector3))
                AdvancedInspectorControl.Field.SetValue(Vector3.zero);
            else if (AdvancedInspectorControl.Field.Type == typeof(Vector4))
                AdvancedInspectorControl.Field.SetValue(Vector4.zero);
        }

        private void One()
        {
            if (AdvancedInspectorControl.Field.Type == typeof(Vector2))
                AdvancedInspectorControl.Field.SetValue(Vector2.one);
            else if (AdvancedInspectorControl.Field.Type == typeof(Vector3))
                AdvancedInspectorControl.Field.SetValue(Vector3.one);
            else if (AdvancedInspectorControl.Field.Type == typeof(Vector4))
                AdvancedInspectorControl.Field.SetValue(Vector4.one);
        }
    }
}