using UnityEngine;
using UnityEditor;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace AdvancedInspector
{
    public class EnumEditor : FieldEditor
    {
        public override bool EditDerived
        {
            get { return true; }
        }

        public override bool Expandable
        {
            get { return false; }
        }

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Enum) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            EnumAttribute display = field.GetAttribute<EnumAttribute>();

            EditorGUI.showMixedValue = field.Mixed;

            EditorGUI.BeginChangeCheck();

            object result = null;
            long value = Convert.ToInt64(GetValue(field));
            if (display == null || !display.Masked)
            {
                if (display == null || display.Display == EnumDisplay.DropDown)
                    result = DrawDropDown(field.Type, value, style, false);
                else if (display.Display == EnumDisplay.Button)
                    result = DrawEnum(field.Type, value, display.MaxItemsPerRow, style == null ? EditorStyles.toolbarButton : style);
                else if (display.Display == EnumDisplay.Checkbox)
                    result = DrawEnum(field.Type, value, display.MaxItemsPerRow, style == null ? EditorStyles.toggle : style);
            }
            else
            {
                if (display == null || display.Display == EnumDisplay.DropDown)
                    result = DrawDropDown(field.Type, value, style, true);
                else if (display.Display == EnumDisplay.Button)
                    result = DrawMasked(field.Type, value, display.MaxItemsPerRow, style == null ? EditorStyles.toolbarButton : style);
                else if (display.Display == EnumDisplay.Checkbox)
                    result = DrawMasked(field.Type, value, display.MaxItemsPerRow, style == null ? EditorStyles.toggle : style);
            }

            if (EditorGUI.EndChangeCheck())
                field.SetValue(result);
        }

        private int SelectedIndex(Array values, long value)
        {
            for (int i = 0; i < values.Length; i++)
                if (Convert.ToInt64(values.GetValue(i)) == value)
                    return i;

            return 0;
        }

        private string[] GetNames(Type type)
        {
            Array values = Enum.GetValues(type);
            List<string> names = Enum.GetNames(type).ToList();

            for (int i = 0; i < names.Count; i++)
            {
                DescriptorAttribute descriptor = ((Enum)values.GetValue(i)).GetAttribute<DescriptorAttribute>();
                if (descriptor != null && !string.IsNullOrEmpty(descriptor.Name))
                    names[i] = descriptor.Name;
                else
                    names[i] = ObjectNames.NicifyVariableName(names[i]);
            }

            return names.ToArray();
        }

        private object DrawDropDown(Type type, long value, GUIStyle style, bool masked)
        {
            string[] names = GetNames(type);
            Array values = Enum.GetValues(type);

            if (masked)
            {
                if (style == null)
                    value = EditorGUILayout.MaskField(Convert.ToInt32(value), names);
                else
                    value = EditorGUILayout.MaskField(Convert.ToInt32(value), names, style);

                return Enum.ToObject(type, value);
            }
            else
            {
                int selected = SelectedIndex(values, value);

                if (style == null)
                    selected = EditorGUILayout.Popup(selected, names);
                else
                    selected = EditorGUILayout.Popup(selected, names, style);

                return Enum.ToObject(type, values.GetValue(selected));
            }
        }

        private object DrawEnum(Type type, long value, int max, GUIStyle style)
        {
            if (max < 1)
                max = 6;

            string[] names = GetNames(type);
            Array values = Enum.GetValues(type);

            int rows = Mathf.CeilToInt((float)names.Length / (float)max);
            int count = (names.Length / rows);
            if (count * rows < names.Length)
                count++;

            int selected = SelectedIndex(values, value);

            GUILayout.BeginVertical();

            for (int i = 0; i < rows; i++)
            {
                GUILayout.BeginHorizontal();

                for (int j = count * i; j < count * (i + 1); j++)
                {
                    if (j >= names.Length)
                        break;

                    if (selected == j)
                        GUILayout.Toggle(true, names[j], style);
                    else if (GUILayout.Toggle(false, names[j], style))
                        selected = j;
                }

                GUILayout.EndHorizontal();
            }

            GUILayout.EndVertical();

            return Enum.ToObject(type, values.GetValue(selected));
        }

        private object DrawMasked(Type type, long value, int max, GUIStyle style)
        {
            if (max < 1)
                max = 6;

            Array values = Enum.GetValues(type);
            string[] names = GetNames(type);

            int rows = Mathf.CeilToInt((float)names.Length / (float)max);
            int count = (names.Length / rows);
            if (count * rows < names.Length)
                count++;

            int result = 0;

            GUILayout.BeginVertical();

            for (int i = 0; i < rows; i++)
            {
                GUILayout.BeginHorizontal();

                for (int j = count * i; j < count * (i + 1); j++)
                {
                    if (j >= names.Length)
                        break;

                    int v = (int)values.GetValue(j);
                    if (GUILayout.Toggle(((int)value & v) == v, names[j], style))
                        result |= v;
                }

                GUILayout.EndHorizontal();
            }

            GUILayout.EndVertical();

            return Enum.ToObject(type, result);
        }
    }
}