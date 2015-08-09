using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class RangeIntEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(RangeInt) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            RangeValueAttribute range = field.GetAttribute<RangeValueAttribute>();
            if (range == null)
                return;

            EditorGUILayout.BeginHorizontal();

            RangeInt[] ranges = field.GetValues<RangeInt>();

            int[] mins = new int[ranges.Length];
            int[] maxs = new int[ranges.Length];
            int min = ranges[0].min;
            int max = ranges[0].max;
            bool different = false;

            for (int i = 0; i < ranges.Length; i++)
            {
                mins[i] = ranges[i].min;
                maxs[i] = ranges[i].max;
                if (ranges[i].min != min || ranges[0].max != max)
                    different = true;
            }

            if (IntegerEditor.DrawInt("", mins, out min, GUILayout.Width(64)))
                for (int i = 0; i < field.Instances.Length; i++)
                    field.SetValue(field.Instances[i], new RangeInt(min, ranges[i].max));

            EditorGUI.BeginChangeCheck();
            float fMin = min;
            float fMax = max;
            EditorGUI.showMixedValue = different;
            EditorGUILayout.MinMaxSlider(ref fMin, ref fMax, range.Min, range.Max);
            if (EditorGUI.EndChangeCheck() && min < max)
                for (int i = 0; i < field.Instances.Length; i++)
                    field.SetValue(field.Instances[i], new RangeInt((int)fMin, (int)fMax));

            if (IntegerEditor.DrawInt("", maxs, out max, GUILayout.Width(64)))
                for (int i = 0; i < field.Instances.Length; i++)
                    field.SetValue(field.Instances[i], new RangeInt(ranges[i].min, max));

            EditorGUILayout.EndHorizontal();
        }
    }

    public class RangeFloatEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(RangeFloat) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            RangeValueAttribute range = field.GetAttribute<RangeValueAttribute>();
            if (range == null)
                return;

            EditorGUILayout.BeginHorizontal();

            RangeFloat[] ranges = field.GetValues<RangeFloat>();

            float[] mins = new float[ranges.Length];
            float[] maxs = new float[ranges.Length];
            float min = ranges[0].min;
            float max = ranges[0].max;
            bool different = false;

            for (int i = 0; i < ranges.Length; i++)
            {
                mins[i] = ranges[i].min;
                maxs[i] = ranges[i].max;
                if (ranges[i].min != min || ranges[0].max != max)
                    different = true;
            }

            if (FloatEditor.DrawFloat("", mins, out min, GUILayout.Width(64)))
                for (int i = 0; i < field.Instances.Length; i++)
                    field.SetValue(field.Instances[i], new RangeFloat(min, ranges[i].max));

            EditorGUI.BeginChangeCheck();
            EditorGUI.showMixedValue = different;
            EditorGUILayout.MinMaxSlider(ref min, ref max, range.Min, range.Max);
            if (EditorGUI.EndChangeCheck() && min < max)
                for (int i = 0; i < field.Instances.Length; i++)
                    field.SetValue(field.Instances[i], new RangeFloat(min, max));

            if (FloatEditor.DrawFloat("", maxs, out max, GUILayout.Width(64)))
                for (int i = 0; i < field.Instances.Length; i++)
                    field.SetValue(field.Instances[i], new RangeFloat(ranges[i].min, max));

            EditorGUILayout.EndHorizontal();
        }
    }
}