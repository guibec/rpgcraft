using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class RectEditor : FieldEditor
    {
        public override bool Expandable
        {
            get { return false; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Rect), typeof(RectOffset) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            Type type = field.BaseType;

            float labelWidth = EditorGUIUtility.labelWidth;
            EditorGUIUtility.labelWidth = VECTOR_FIELD_WIDTH;

            if (type == typeof(Rect))
            {
                Rect[] values = field.GetValues<Rect>();

                float[] x = new float[values.Length];
                float[] y = new float[values.Length];
                float[] height = new float[values.Length];
                float[] width = new float[values.Length];

                for (int i = 0; i < values.Length; i++)
                {
                    x[i] = values[i].x;
                    y[i] = values[i].y;
                    height[i] = values[i].height;
                    width[i] = values[i].width;
                }

                GUILayout.BeginHorizontal();
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
                GUILayout.EndHorizontal();

                GUILayout.BeginHorizontal();
                if (FloatEditor.DrawFloat("W", width, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Width");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].width = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (FloatEditor.DrawFloat("H", height, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Height");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].height = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }
                GUILayout.EndHorizontal();
            }
            else if (type == typeof(RectOffset))
            {
                RectOffset[] values = field.GetValues<RectOffset>();

                int[] left = new int[values.Length];
                int[] right = new int[values.Length];
                int[] top = new int[values.Length];
                int[] bottom = new int[values.Length];

                for (int i = 0; i < values.Length; i++)
                {
                    left[i] = values[i].left;
                    right[i] = values[i].right;
                    top[i] = values[i].top;
                    bottom[i] = values[i].bottom;
                }

                GUILayout.BeginHorizontal();
                int result;
                if (IntegerEditor.DrawInt("L", left, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Left");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].left = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (IntegerEditor.DrawInt("R", right, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Right");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].right = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }
                GUILayout.EndHorizontal();

                GUILayout.BeginHorizontal();
                if (IntegerEditor.DrawInt("T", top, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Top");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].top = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }

                if (IntegerEditor.DrawInt("B", bottom, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Bottom");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        values[i].bottom = result;
                        field.SetValue(field.Instances[i], values[i]);
                    }
                }
                GUILayout.EndHorizontal();
            }

            EditorGUILayout.Space();

            EditorGUIUtility.labelWidth = labelWidth;
        }
    }
}