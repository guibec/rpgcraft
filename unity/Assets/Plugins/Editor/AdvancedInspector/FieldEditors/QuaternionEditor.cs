using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class QuaternionEditor : FieldEditor
    {
        public override bool Expandable
        {
            get { return false; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Quaternion) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            float width = EditorGUIUtility.labelWidth;
            EditorGUIUtility.labelWidth = VECTOR_FIELD_WIDTH;

            Quaternion[] values = field.GetValues<Quaternion>();
            if (AdvancedInspectorControl.Level <= InspectorLevel.Advanced)
            {
                float[] x = new float[values.Length];
                float[] y = new float[values.Length];
                float[] z = new float[values.Length];

                for (int i = 0; i < values.Length; i++)
                {
                    Vector3 euler = values[i].eulerAngles;
                    x[i] = euler.x;
                    y[i] = euler.y;
                    z[i] = euler.z;
                }

                GUILayout.BeginHorizontal();

                float result;
                if (FloatEditor.DrawFloat("X", x, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " X");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Vector3 v = field.GetValue<Quaternion>(field.Instances[i]).eulerAngles;
                        v.x = result;
                        field.SetValue(field.Instances[i], Quaternion.Euler(v));
                    }
                }

                if (FloatEditor.DrawFloat("Y", y, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Y");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Vector3 v = field.GetValue<Quaternion>(field.Instances[i]).eulerAngles;
                        v.y = result;
                        field.SetValue(field.Instances[i], Quaternion.Euler(v));
                    }
                }

                if (FloatEditor.DrawFloat("Z", z, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Z");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Vector3 v = field.GetValue<Quaternion>(field.Instances[i]).eulerAngles;
                        v.z = result;
                        field.SetValue(field.Instances[i], Quaternion.Euler(v));
                    }
                }

                GUILayout.EndHorizontal();
            }
            else
            {
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

                GUILayout.BeginHorizontal();

                float result;
                if (FloatEditor.DrawFloat("X", x, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " X");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Quaternion v = field.GetValue<Quaternion>(field.Instances[i]);
                        v.x = result;
                        field.SetValue(field.Instances[i], v);
                    }
                }

                if (FloatEditor.DrawFloat("Y", y, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Y");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Quaternion v = field.GetValue<Quaternion>(field.Instances[i]);
                        v.y = result;
                        field.SetValue(field.Instances[i], v);
                    }
                }

                GUILayout.EndHorizontal();
                GUILayout.BeginHorizontal();

                if (FloatEditor.DrawFloat("Z", z, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Z");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Quaternion v = field.GetValue<Quaternion>(field.Instances[i]);
                        v.z = result;
                        field.SetValue(field.Instances[i], v);
                    }
                }

                if (FloatEditor.DrawFloat("W", w, style, out result))
                {
                    Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " W");

                    for (int i = 0; i < field.Instances.Length; i++)
                    {
                        Quaternion v = field.GetValue<Quaternion>(field.Instances[i]);
                        v.w = result;
                        field.SetValue(field.Instances[i], v);
                    }
                }

                GUILayout.EndHorizontal();
            }

            EditorGUIUtility.labelWidth = width;
        }

        public override void OnContextualClick(InspectorField field, GenericMenu menu)
        {
            menu.AddItem(new GUIContent("Identity"), false, Identity);

            menu.AddSeparator("");
        }

        private void Identity()
        {
            AdvancedInspectorControl.Field.SetValue(Quaternion.identity);
        }
    }
}