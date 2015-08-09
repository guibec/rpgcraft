using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(ConstantForce), true)]
    public class ConstantForceEditor : JointEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(ConstantForce);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("force"),
                new DescriptorAttribute("Force", "The force applied to the rigidbody every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-force.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("relativeForce"),
                new DescriptorAttribute("Relative Force", "The force - relative to the rigid bodies coordinate system - applied every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-relativeForce.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("torque"),
                new DescriptorAttribute("Torque", "The torque applied to the rigidbody every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-torque.html"))); 
            fields.Add(new InspectorField(type, Instances, type.GetProperty("relativeTorque"),
                new DescriptorAttribute("Relative Torque", "The torque - relative to the rigid bodies coordinate system - applied every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-relativeTorque.html")));
        }
    }
}