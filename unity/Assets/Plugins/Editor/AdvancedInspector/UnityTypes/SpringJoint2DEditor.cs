using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(SpringJoint2D), true)]
    public class SpringJoint2DEditor : AnchoredJoint2DEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(SpringJoint2D);
            
            fields.Add(new InspectorField(type, Instances, type.GetProperty("dampingRatio"),
                new DescriptorAttribute("Damping Ratio", "The amount by which the spring force is reduced in proportion to the movement speed.", "http://docs.unity3d.com/ScriptReference/SpringJoint2D-dampingRatio.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("distance"),
                new DescriptorAttribute("Distance", "The distance the spring will try to keep between the two objects.", "http://docs.unity3d.com/ScriptReference/SpringJoint2D-distance.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("frequency"),
                new DescriptorAttribute("Frequency", "The frequency at which the spring oscillates around the distance distance between the objects.", "http://docs.unity3d.com/ScriptReference/SpringJoint2D-frequency.html"))); 
        }
    }
}