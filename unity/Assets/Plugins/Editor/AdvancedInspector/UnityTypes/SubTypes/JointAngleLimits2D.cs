using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

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

            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("min"),
                new DescriptorAttribute("Lower Angle", "Lower angular limit of rotation.", "http://docs.unity3d.com/ScriptReference/JointAngleLimits2D-min.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("max"),
                new DescriptorAttribute("Upper Angle", "Upper angular limit of rotation.", "http://docs.unity3d.com/ScriptReference/JointAngleLimits2D-max.html")));
        }
    }
}