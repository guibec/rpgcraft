using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using UnityEditor;

namespace AdvancedInspector
{
    public static class ExtraGUILayout
    {
        public static float FloatAngle(float value)
        {
            return FloatAngle(value, -1, -1, -1);
        }

        public static float FloatAngle(float value, float snap)
        {
            return FloatAngle(value, snap, -1, -1);
        }

        public static float FloatAngle(float value, float snap, float min, float max)
        {
            Rect rect = GUILayoutUtility.GetRect(128, 512, 32, 32);
            rect.x += 8;
            return ExtraGUI.FloatAngle(rect, value, snap, min, max);
        }

        public static int IntAngle(int value)
        {
            return IntAngle(value, -1, -1, -1);
        }

        public static int IntAngle(int value, int snap)
        {
            return IntAngle(value, snap, -1, -1);
        }

        public static int IntAngle(int value, int snap, int min, int max)
        {
            Rect rect = GUILayoutUtility.GetRect(128, 512, 32, 32);
            rect.x += 8;
            return ExtraGUI.IntAngle(rect, value, snap, min, max);
        }

        public static int CycleButton(int selected, GUIContent[] contents, GUIStyle style)
        {
            if (GUILayout.Button(contents[selected], style))
            {
                selected++;
                if (selected >= contents.Length)
                    selected = 0;
            }

            return selected;
        }
    }
}
