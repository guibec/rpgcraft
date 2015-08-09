using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class IntegerEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(int), typeof(byte), typeof(short), typeof(long), typeof(uint), typeof(ushort), typeof(ulong), typeof(sbyte) }; }
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
                long value = Convert.ToInt64(result);
                result = (long)Math.Round(value + (CalculateDragSensitivity(value) * HandleUtility.niceMouseDelta * 0.1f));

                RangeValueAttribute rangeValue = field.GetAttribute<RangeValueAttribute>();
                if (rangeValue != null)
                    result = Math.Min(Math.Max((long)result, (long)rangeValue.Min), (long)rangeValue.Max);

                RangeAttribute range = field.GetAttribute<RangeAttribute>();
                if (range != null)
                    result = Math.Min(Math.Max((long)result, (long)range.min), (long)range.max);

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
            return Math.Max(1, Math.Pow(Math.Abs((long)value), 0.5d) * 0.03d);
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            AngleAttribute angle = field.GetAttribute<AngleAttribute>();

            RangeValueAttribute rangeValue = field.GetAttribute<RangeValueAttribute>();
            RangeAttribute range = field.GetAttribute<RangeAttribute>();

            if (range != null && rangeValue == null)
                rangeValue = new RangeValueAttribute(range.min, range.max);

            object result;
            if (DrawIntegerNumber(GUIContent.none, field.GetValues(), rangeValue, angle, style, out result))
                field.SetValue(result);
        }

        public static bool DrawInt(string label, int[] values, out int result, params GUILayoutOption[] options)
        {
            return DrawInt(label, values, null, out result, options);
        }

        public static bool DrawInt(string label, int[] values, GUIStyle style, out int result, params GUILayoutOption[] options)
        {
            object genericResult;
            object[] genericValues = new object[values.Length];
            for (int i = 0; i < values.Length; i++)
                genericValues[i] = values[i];

            bool changed = DrawIntegerNumber(new GUIContent(label), genericValues, null, null, style, out genericResult, options);

            try
            {
                result = (int)genericResult;
            }
            catch (Exception)
            {
                result = values[0];
                return false;
            }

            return changed;
        }

        public static bool DrawIntegerNumber(GUIContent label, object[] values, RangeValueAttribute range, AngleAttribute angle, GUIStyle style, out object result, params GUILayoutOption[] options)
        {
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
                        long value = Convert.ToInt64(result);
                        result = (long)Math.Round(value + (CalculateDragSensitivity(value) * HandleUtility.niceMouseDelta * 0.1f));
                        if (range != null)
                            result = Math.Min(Math.Max((long)result, (long)range.Min), (long)range.Max);

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
                    result = ExtraGUILayout.IntAngle((int)result, (int)angle.Snap, (int)range.Min, (int)range.Max);
                else
                    result = ExtraGUILayout.IntAngle((int)result, (int)angle.Snap);
            }
            else
            {
                if (range != null)
                    result = EditorGUILayout.IntSlider((int)result, (int)range.Min, (int)range.Max, options);
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