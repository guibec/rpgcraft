using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(JointLimits), true)]
    public class JointLimitsEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(JointLimits);

            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("min"),
                new DescriptorAttribute("Minimum", "The lower limit of the joint.", "http://docs.unity3d.com/ScriptReference/JointLimits-min.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("minBounce"),
                new DescriptorAttribute("Minimum Bounce", "The bounciness of the joint when hitting the lower limit of the joint.", "http://docs.unity3d.com/ScriptReference/JointLimits-minBounce.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("max"),
                new DescriptorAttribute("Maximum", "The upper limit of the joint.", "http://docs.unity3d.com/ScriptReference/JointLimits-max.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("maxBounce"),
                new DescriptorAttribute("Maximum Bounce", "The bounciness of the joint when hitting the upper limit of the joint.", "http://docs.unity3d.com/ScriptReference/JointLimits-maxBounce.html")));
        }
    }
}