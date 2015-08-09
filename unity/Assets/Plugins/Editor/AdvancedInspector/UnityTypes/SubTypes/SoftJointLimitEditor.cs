using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

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

            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("bounciness"),
                new DescriptorAttribute("Bounciness", "When the joint hits the limit, it can be made to bounce off it.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-bounciness.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("damper"),
                new DescriptorAttribute("Damper", "If spring is greater than zero, the limit is soft.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-damper.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("limit"),
                new DescriptorAttribute("Limit", "The limit position/angle of the joint.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-limit.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("spring"),
                new DescriptorAttribute("Spring", "If greater than zero, the limit is soft. The spring will pull the joint back.", "http://docs.unity3d.com/ScriptReference/SoftJointLimit-spring.html")));
        }
    }
}