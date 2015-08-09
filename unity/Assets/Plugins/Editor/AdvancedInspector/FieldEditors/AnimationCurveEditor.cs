using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class AnimationCurveEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(AnimationCurve) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            AnimationCurve curve = GetValue(field) as AnimationCurve;
            if (curve == null && !field.Mixed)
                curve = new AnimationCurve();

            EditorGUI.BeginChangeCheck();

            AnimationCurve result = EditorGUILayout.CurveField(curve);

            if (EditorGUI.EndChangeCheck())
                field.SetValue(result);
        }
    }
}
