using UnityEngine;
using UnityEditor;
using System;
using System.Collections;
using System.Collections.Generic;

namespace AdvancedInspector
{
    public class LayerMaskEditor : FieldEditor
    {
        private static List<string> names;
        private static List<int> masks;

        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(LayerMask) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            names = new List<string>();
            masks = new List<int>();

            for (int i = 0; i < 32; i++)
            {
                string name = LayerMask.LayerToName(i);

                if (!string.IsNullOrEmpty(name))
                {
                    names.Add(name);
                    masks.Add(1 << i);
                }
            }

            object value = field.GetValue();

            if (value != null)
            {
                bool isMask = value is LayerMask;

                int mask;
                if (isMask)
                    mask = (int)(LayerMask)value;
                else
                    mask = (int)value;

                int result;
                if (DrawLayerMaskField(mask, style, out result))
                {
                    if (isMask)
                        field.SetValue((LayerMask)result);
                    else
                        field.SetValue(result);
                }
            }
            else
            {
                EditorGUI.showMixedValue = true;

                int result;
                if (DrawLayerMaskField(0, style, out result))
                {
                    field.SetValue(result);
                }
            }
        }

        public static bool DrawLayerMaskField(int aMask, GUIStyle style, out int result)
        {
            int val = aMask;
            int maskVal = 0;
            for (int i = 0; i < names.Count; i++)
            {
                if (masks[i] != 0)
                {
                    if ((val & masks[i]) == masks[i])
                        maskVal |= 1 << i;
                }
                else if (val == 0)
                    maskVal |= 1 << i;
            }

            EditorGUI.BeginChangeCheck();

            int newMaskVal;
            if (style != null)
                newMaskVal = EditorGUILayout.MaskField(maskVal, names.ToArray(), style);
            else
                newMaskVal = EditorGUILayout.MaskField(maskVal, names.ToArray());

            int changes = maskVal ^ newMaskVal;

            for (int i = 0; i < masks.Count; i++)
            {
                if ((changes & (1 << i)) != 0)
                {
                    if ((newMaskVal & (1 << i)) != 0)
                    {
                        if (masks[i] == 0)
                        {
                            val = 0;
                            break;
                        }
                        else
                            val |= masks[i];
                    }
                    else
                        val &= ~masks[i];
                }
            }

            result = val;

            return EditorGUI.EndChangeCheck();
        }
    }
}