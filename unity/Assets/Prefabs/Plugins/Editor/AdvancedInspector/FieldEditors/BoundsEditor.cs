using UnityEngine;
using UnityEditor;
using System;

namespace AdvancedInspector
{
    public class BoundsEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Bounds) }; }
        }

        public override bool IsExpandable(InspectorField field)
        {
            return false;
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            float labelWidth = EditorGUIUtility.labelWidth;
            EditorGUIUtility.labelWidth = VECTOR_FIELD_WIDTH;

            Bounds[] values = field.GetValues<Bounds>();

            float[] centerX = new float[values.Length];
            float[] centerY = new float[values.Length];
            float[] centerZ = new float[values.Length];
            float[] extendX = new float[values.Length];
            float[] extendY = new float[values.Length];
            float[] extendZ = new float[values.Length];

            for (int i = 0; i < values.Length; i++)
            {
                centerX[i] = values[i].center.x;
                centerY[i] = values[i].center.y;
                centerZ[i] = values[i].center.z;
                extendX[i] = values[i].extents.x;
                extendY[i] = values[i].extents.y;
                extendZ[i] = values[i].extents.z;
            }

            GUILayout.BeginHorizontal();
            float result;
            EditorGUILayout.LabelField("Center: ", GUILayout.Width(48));
            if (FloatEditor.DrawFloat("X", centerX, style, out result))
            {
                field.RecordObjects("Edit " + field.Name + " Center X");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    Vector3 center = values[i].center;
                    center.x = result;
                    values[i].center = center;
                    field.SetValue(field.Instances[i], values[i]);
                }
            }

            if (FloatEditor.DrawFloat("Y", centerY, style, out result))
            {
                field.RecordObjects("Edit " + field.Name + " Center Y");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    Vector3 center = values[i].center;
                    center.y = result;
                    values[i].center = center;
                    field.SetValue(field.Instances[i], values[i]);
                }
            }

            if (FloatEditor.DrawFloat("Z", centerZ, style, out result))
            {
                field.RecordObjects("Edit " + field.Name + " Center Z");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    Vector3 center = values[i].center;
                    center.z = result;
                    values[i].center = center;
                    field.SetValue(field.Instances[i], values[i]);
                }
            }
            GUILayout.EndHorizontal();

            GUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Extend: ", GUILayout.Width(48));
            if (FloatEditor.DrawFloat("X", extendX, style, out result))
            {
                field.RecordObjects("Edit " + field.Name + " Extend X");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    Vector3 extents = values[i].extents;
                    extents.x = result;
                    values[i].extents = extents;
                    field.SetValue(field.Instances[i], values[i]);
                }
            }

            if (FloatEditor.DrawFloat("Y", extendY, style, out result))
            {
                field.RecordObjects("Edit " + field.Name + " Extend Y");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    Vector3 extents = values[i].extents;
                    extents.y = result;
                    values[i].extents = extents;
                    field.SetValue(field.Instances[i], values[i]);
                }
            }

            if (FloatEditor.DrawFloat("Z", extendZ, style, out result))
            {
                field.RecordObjects("Edit " + field.Name + " Extend Z");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    Vector3 extents = values[i].extents;
                    extents.z = result;
                    values[i].extents = extents;
                    field.SetValue(field.Instances[i], values[i]);
                }
            }
            GUILayout.EndHorizontal();

            EditorGUILayout.Space();

            EditorGUIUtility.labelWidth = labelWidth;
        }
    }
}