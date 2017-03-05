using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(JointAngleLimits2D), true)]
    public class JointAngleLimits2DEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(JointAngleLimits2D);

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("min"),
                new DescriptorAttribute("Lower Angle", "Lower angular limit of rotation.", "http://docs.unity3d.com/ScriptReference/JointAngleLimits2D-min.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("max"),
                new DescriptorAttribute("Upper Angle", "Upper angular limit of rotation.", "http://docs.unity3d.com/ScriptReference/JointAngleLimits2D-max.html")));
        }
    }
}