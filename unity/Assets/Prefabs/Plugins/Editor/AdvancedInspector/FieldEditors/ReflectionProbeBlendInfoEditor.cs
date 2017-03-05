using UnityEngine;
using UnityEngine.Rendering;
using UnityEditor;
using System;

namespace AdvancedInspector
{
    public class ReflectionProbeBlendInfoEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(ReflectionProbeBlendInfo) }; }
        }

        public override bool IsExpandable(InspectorField field)
        {
            return false;
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            ReflectionProbeBlendInfo info = field.GetValue<ReflectionProbeBlendInfo>();

            GUILayout.BeginHorizontal();

            EditorGUILayout.ObjectField(info.probe, typeof(ReflectionProbe), true, GUILayout.MinWidth(100), GUILayout.MaxWidth(float.MaxValue));
            GUILayout.Label("Weight: " + info.weight.ToString());

            GUILayout.EndHorizontal();
        }
    }
}