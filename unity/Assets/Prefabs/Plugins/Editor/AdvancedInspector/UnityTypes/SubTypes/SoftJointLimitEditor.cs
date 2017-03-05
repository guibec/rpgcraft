using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(SoftJointLimit), true)]
    public class SoftJointLimitEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(SoftJointLimit);

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("bounciness"),
                new DescriptorAttribute("Bounciness", "When the joint hits the limit, it can be made to bounce off it.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-bounciness.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("limit"),
                new DescriptorAttribute("Limit", "The limit position/angle of the joint.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-limit.html")));
        }
    }
}