using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using UnityEditor;

namespace AdvancedInspector
{
    public static class ExtraGUI
    {
        private static Texture knob;

        private static Texture Knob
        {
            get
            {
                if (knob == null)
                {
                    knob = Helper.Load(EditorResources.Knob);
                    knob.filterMode = FilterMode.Trilinear;
                }

                return knob;
            }
        }

        private static Texture knobBack;

        private static Texture KnobBack
        {
            get
            {
                if (knobBack == null)
                    knobBack = Helper.Load(EditorResources.KnobBack);

                return knobBack;
            }
        }

        private static Vector2 mousePosition;

        public static float FloatAngle(Rect rect, float value)
        {
            return FloatAngle(rect, value, -1, -1, -1);
        }

        public static float FloatAngle(Rect rect, float value, float snap)
        {
            return FloatAngle(rect, value, snap, -1, -1);
        }

        public static float FloatAngle(Rect rect, float value, float snap, float min, float max)
        {
            int id = GUIUtility.GetControlID(FocusType.Native, rect);

            Rect knobRect = new Rect(rect.x, rect.y, rect.height, rect.height);

            float delta;
            if (min != max)
                delta = ((max - min) / 360);
            else
                delta = 1;

            if (Event.current != null)
            {
                if (Event.current.type == EventType.MouseDown && knobRect.Contains(Event.current.mousePosition))
                {
                    GUIUtility.hotControl = id;
                    mousePosition = Event.current.mousePosition;
                }
                else if (Event.current.type == EventType.MouseUp && GUIUtility.hotControl == id)
                    GUIUtility.hotControl = 0;
                else if (Event.current.type == EventType.MouseDrag && GUIUtility.hotControl == id)
                { 
                    Vector2 move = mousePosition - Event.current.mousePosition;
                    value += delta * (-move.x - move.y);

                    if (snap > 0)
                    {
                        float mod = value % snap;

                        if (mod < (delta * 3) || Mathf.Abs(mod - snap) < (delta * 3))
                            value = Mathf.Round(value / snap) * snap;
                    }

                    mousePosition = Event.current.mousePosition;
                    GUI.changed = true;
                }
            }

            GUI.DrawTexture(knobRect, KnobBack);
            Matrix4x4 matrix = GUI.matrix;

            if (min != max)
                GUIUtility.RotateAroundPivot(value * (360 / (max - min)), knobRect.center);
            else
                GUIUtility.RotateAroundPivot(value, knobRect.center);

            GUI.DrawTexture(knobRect, Knob);
            GUI.matrix = matrix;

            Rect label = new Rect(rect.x + rect.height + 9, rect.y + (rect.height / 2) - 9, rect.height, 18);
            value = EditorGUI.FloatField(label, value);

            if (min != max)
                value = Mathf.Clamp(value, min, max);

            return value;
        }

        public static int IntAngle(Rect rect, int value)
        {
            return IntAngle(rect, value, -1, -1, -1);
        }

        public static int IntAngle(Rect rect, int value, int snap)
        {
            return IntAngle(rect, value, snap, -1, -1);
        }

        public static int IntAngle(Rect rect, int value, int snap, int min, int max)
        {
            int id = GUIUtility.GetControlID(FocusType.Native, rect);

            Rect knobRect = new Rect(rect.x, rect.y, rect.height, rect.height);

            int delta;
            if (min != max)
                delta = ((max - min) / 360);
            else
                delta = 1;

            if (Event.current != null)
            {
                if (Event.current.type == EventType.MouseDown && knobRect.Contains(Event.current.mousePosition))
                {
                    GUIUtility.hotControl = id;
                    mousePosition = Event.current.mousePosition;
                }
                else if (Event.current.type == EventType.MouseUp && GUIUtility.hotControl == id)
                    GUIUtility.hotControl = 0;
                else if (Event.current.type == EventType.MouseDrag && GUIUtility.hotControl == id)
                {
                    Vector2 move = mousePosition - Event.current.mousePosition;
                    value += delta * (-(int)move.x - (int)move.y);

                    if (snap > 0)
                    {
                        float mod = value % snap;

                        if (mod < (delta * 3) || Mathf.Abs(mod - snap) < (delta * 3))
                            value = (int)Mathf.Round(value / snap) * snap;
                    }

                    mousePosition = Event.current.mousePosition;
                    GUI.changed = true;
                }
            }

            GUI.DrawTexture(knobRect, KnobBack);
            Matrix4x4 matrix = GUI.matrix;

            if (min != max)
                GUIUtility.RotateAroundPivot(value * (360 / (max - min)), knobRect.center);
            else
                GUIUtility.RotateAroundPivot(value, knobRect.center);

            GUI.DrawTexture(knobRect, Knob);
            GUI.matrix = matrix;

            Rect label = new Rect(rect.x + rect.height + 9, rect.y + (rect.height / 2) - 9, rect.height, 18);
            value = EditorGUI.IntField(label, value);

            if (min != max)
                value = Mathf.Clamp(value, min, max);

            return value;
        }

        public static int CycleButton(Rect rect, int selected, GUIContent[] contents, GUIStyle style)
        {
            if (GUI.Button(rect, contents[selected], style))
            {
                selected++;
                if (selected >= contents.Length)
                    selected = 0;
            }

            return selected;
        }
    }
}
