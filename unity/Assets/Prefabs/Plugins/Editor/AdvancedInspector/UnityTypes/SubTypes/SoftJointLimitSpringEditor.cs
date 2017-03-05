using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(SoftJointLimitSpring), true)]
    public class SoftJointLimitSpringEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(SoftJointLimitSpring);

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("damper"),
                new DescriptorAttribute("Damper", "If spring is greater than zero, the limit is soft.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-damper.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("limit"),
                new DescriptorAttribute("Spring", "If greater than zero, the limit is soft. The spring will pull the joint back.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-spring.html")));
        }
    }
}