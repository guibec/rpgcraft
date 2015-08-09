using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(JointMotor), true)]
    public class JointMotorEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(JointMotor);

            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("force"),
                new DescriptorAttribute("Force", "The motor will apply a force.", "http://docs.unity3d.com/ScriptReference/JointMotor-force.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("freeSpin"),
                new DescriptorAttribute("Free Spin", "If freeSpin is enabled the motor will only accelerate but never slow down.", "http://docs.unity3d.com/ScriptReference/JointMotor-freeSpin.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("targetVelocity"),
                new DescriptorAttribute("Target Velocity", "The motor will apply a force up to force to achieve targetVelocity.", "http://docs.unity3d.com/ScriptReference/JointMotor-targetVelocity.html")));
        }
    }
}