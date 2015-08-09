using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class TimeSpanEditor : FieldEditor
    {
        private InspectorField field;

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(TimeSpan) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            int[] days = new int[field.Instances.Length]; 
            int[] hours = new int[field.Instances.Length]; 
            int[] mins = new int[field.Instances.Length]; 
            int[] secs = new int[field.Instances.Length];
            for (int i = 0; i < field.Instances.Length; i++)
            {
                TimeSpan span = field.GetValue<TimeSpan>(field.Instances[i]);
                days[i] = span.Days;
                hours[i] = span.Hours;
                mins[i] = span.Minutes;
                secs[i] = span.Seconds;
            }

            float labelWidth = EditorGUIUtility.labelWidth;
            EditorGUIUtility.labelWidth = 42;
            GUILayout.BeginHorizontal();

            int result;
            if (IntegerEditor.DrawInt("Days", days, style, out result))
            {
                result = Mathf.Clamp(result, 0, int.MaxValue);
                Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Days");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    TimeSpan span = field.GetValue<TimeSpan>(field.Instances[i]);
                    span = new TimeSpan(result, span.Hours, span.Minutes, span.Seconds);
                    field.SetValue(field.Instances[i], span);
                }
            }

            if (IntegerEditor.DrawInt("Hours", hours, style, out result))
            {
                result = Mathf.Clamp(result, 0, int.MaxValue);
                Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Hours");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    TimeSpan span = field.GetValue<TimeSpan>(field.Instances[i]);
                    span = new TimeSpan(span.Days, result, span.Minutes, span.Seconds);
                    field.SetValue(field.Instances[i], span);
                }
            }

            GUILayout.EndHorizontal();
            GUILayout.BeginHorizontal();

            if (IntegerEditor.DrawInt("Mins", mins, style, out result))
            {
                result = Mathf.Clamp(result, 0, int.MaxValue);
                Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Minutes");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    TimeSpan span = field.GetValue<TimeSpan>(field.Instances[i]);
                    span = new TimeSpan(span.Days, span.Hours, result, span.Seconds);
                    field.SetValue(field.Instances[i], span);
                }
            }

            if (IntegerEditor.DrawInt("Secs", secs, style, out result))
            {
                result = Mathf.Clamp(result, 0, int.MaxValue);
                Undo.RecordObjects(field.SerializedInstances, "Edit " + field.Name + " Seconds");

                for (int i = 0; i < field.Instances.Length; i++)
                {
                    TimeSpan span = field.GetValue<TimeSpan>(field.Instances[i]);
                    span = new TimeSpan(span.Days, span.Hours, span.Minutes, result);
                    field.SetValue(field.Instances[i], span);
                }
            }

            GUILayout.EndHorizontal();
            EditorGUILayout.Space();

            EditorGUIUtility.labelWidth = labelWidth;
        }
    }
}