using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(SpringJoint), true)]
    public class SpringJointEditor : JointEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(SpringJoint);
            
            fields.Add(new InspectorField(type, Instances, type.GetProperty("damper"),
                new DescriptorAttribute("Damper", "The damper force used to dampen the spring force.", "http://docs.unity3d.com/ScriptReference/SpringJoint-damper.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("spring"),
                new DescriptorAttribute("Spring", "The spring force used to keep the two objects together.", "http://docs.unity3d.com/ScriptReference/SpringJoint-spring.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("minDistance"),
                new DescriptorAttribute("Minimum Distance", "The minimum distance between the bodies relative to their initial distance.", "http://docs.unity3d.com/ScriptReference/SpringJoint-minDistance.html"))); 
            fields.Add(new InspectorField(type, Instances, type.GetProperty("maxDistance"),
                new DescriptorAttribute("Maximum Distance", "The maximum distance between the bodies relative to their initial distance.", "http://docs.unity3d.com/ScriptReference/SpringJoint-maxDistance.html")));
        }
    }
}