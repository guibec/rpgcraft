using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class FloatEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(float), typeof(decimal), typeof(double) }; }
        }

        public override void OnLabelDraw(InspectorField field, Rect rect)
        {
            if (InspectorPreferences.IsDragControl(InspectorPreferences.ValueScroll))
                EditorGUIUtility.AddCursorRect(rect, MouseCursor.ResizeHorizontal);
        }

        public override void OnLabelClick(InspectorField field)
        {
            EditorGUIUtility.SetWantsMouseJumping(1);
        }

        public override void OnLabelDragged(InspectorField field)
        {
            if (InspectorPreferences.IsDragControl(InspectorPreferences.ValueScroll))
            {
                object result = field.GetValues()[0];
                double sensitivity = CalculateDragSensitivity(result);
                result = Convert.ToDouble(result) + (double)(sensitivity * HandleUtility.niceMouseDelta);

                RangeValueAttribute rangeValue = field.GetAttribute<RangeValueAttribute>();
                if (rangeValue != null)
                    result = Math.Min(Math.Max((double)result, rangeValue.Min), rangeValue.Max);

                RangeAttribute range = field.GetAttribute<RangeAttribute>();
                if (range != null)
                    result = Math.Min(Math.Max((double)result, range.min), range.max);

                try
                {
                    result = Convert.ChangeType(result, field.Type);
                    field.SetValue(result);
                }
                catch (Exception)
                {
                    return;
                }
            }
        }

        private static double CalculateDragSensitivity(object value)
        {
            double number = Convert.ToDouble(value);

            if (!double.IsInfinity(number) && !double.IsNaN(number))
                return (Math.Max(1d, Math.Pow(Math.Abs(number), 0.5d)) * 0.03d);

            return 0d;
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            RangeValueAttribute rangeValue = field.GetAttribute<RangeValueAttribute>();
            RangeAttribute range = field.GetAttribute<RangeAttribute>();

            if (range != null && rangeValue == null)
                rangeValue = new RangeValueAttribute(range.min, range.max);

            AngleAttribute angle = field.GetAttribute<AngleAttribute>();

            object result;
            if (DrawFloatingNumber(new GUIContent(""), field.GetValues(), rangeValue, angle, style, out result))
                field.SetValue(result);
        }

        public static bool DrawFloat(string label, float[] values, out float result, params GUILayoutOption[] options)
        {
            return DrawFloat(label, values, null, null, null, out result, options);
        }

        public static bool DrawFloat(string label, float[] values, GUIStyle style, out float result, params GUILayoutOption[] options)
        {
            return DrawFloat(label, values, null, null, style, out result, options);
        }

        public static bool DrawFloat(string label, float[] values, RangeValueAttribute range, AngleAttribute angle, GUIStyle style, out float result, params GUILayoutOption[] options)
        {
            object genericResult;
            object[] genericValues = new object[values.Length];
            for (int i = 0; i < values.Length; i++)
                genericValues[i] = values[i];

            bool changed = DrawFloatingNumber(new GUIContent(label), genericValues, range, angle, style, out genericResult, options);

            try
            {
                result = (float)genericResult;
            }
            catch (Exception)
            {
                result = values[0];
                return false;
            }

            return changed;
        }

        public static bool DrawFloatingNumber(GUIContent label, object[] values, RangeValueAttribute range, AngleAttribute angle, GUIStyle style, out object result, params GUILayoutOption[] options)
        {
            result = 0;

            EditorGUI.showMixedValue = false;
            result = values[0];
            Type type = result.GetType();
            for (int i = 1; i < values.Length; i++)
            {
                if (values[i].Equals(result))
                    continue;

                EditorGUI.showMixedValue = true;
                break;
            }

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.BeginHorizontal();

            if (!string.IsNullOrEmpty(label.text))
            {
                int size = (int)GUI.skin.label.CalcSize(label).x;
                if (size > 4)
                    size = Mathf.Max(size, 15);

                GUILayout.Label(label, GUI.skin.label, GUILayout.Width(size));

                Event e = Event.current;
                Rect labelRect = GUILayoutUtility.GetLastRect();
                int id = EditorGUIUtility.GetControlID(FocusType.Native, labelRect);

                if (InspectorPreferences.IsDragControl(InspectorPreferences.ValueScroll))
                {
                    if (e.type == EventType.Repaint)
                    {
                        EditorGUIUtility.AddCursorRect(labelRect, MouseCursor.ResizeHorizontal);
                    }
                    else if (e.type == EventType.MouseDown && labelRect.Contains(e.mousePosition) && e.button == 0) 
                    {
                        GUIUtility.hotControl = id;
                        GUIUtility.keyboardControl = id;
                        EditorGUIUtility.SetWantsMouseJumping(1);
                        e.Use();
                    }
                    else if (e.type == EventType.MouseDrag && GUIUtility.hotControl == id)
                    {
                        double sensitivity = CalculateDragSensitivity(result);
                        result = Convert.ToDecimal(result) + (decimal)(sensitivity * HandleUtility.niceMouseDelta);

                        if (range != null)
                            result = Math.Min(Math.Max((double)result, (double)range.Min), (double)range.Max);

                        GUI.changed = true;
                        e.Use();
                    }
                    else if (e.type == EventType.MouseUp && GUIUtility.hotControl == id)
                    {
                        GUIUtility.hotControl = 0;
                        EditorGUIUtility.SetWantsMouseJumping(0);
                        e.Use();
                    }
                }
            }

            if (angle != null)
            {
                if (range != null)
                    result = ExtraGUILayout.FloatAngle((float)result, angle.Snap, range.Min, range.Max);
                else
                    result = ExtraGUILayout.FloatAngle((float)result, angle.Snap);
            }
            else
            {
                if (range != null)
                    result = EditorGUILayout.Slider((float)result, range.Min, range.Max, options);
                else
                { 
                    if (style != null)
                        result = EditorGUILayout.TextField(result.ToString(), style, options);
                    else
                        result = EditorGUILayout.TextField(result.ToString(), options);
                }
            }

            try
            {
                result = Convert.ChangeType(result, type);
            }
            catch (Exception)
            {
                return false;
            }

            EditorGUILayout.EndHorizontal();
            return EditorGUI.EndChangeCheck();
        }
    }
}