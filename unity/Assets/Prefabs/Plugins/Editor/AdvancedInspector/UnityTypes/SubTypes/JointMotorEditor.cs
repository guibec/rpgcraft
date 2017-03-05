using System;

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

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("force"),
                new DescriptorAttribute("Force", "The motor will apply a force.", "http://docs.unity3d.com/ScriptReference/JointMotor-force.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("freeSpin"),
                new DescriptorAttribute("Free Spin", "If freeSpin is enabled the motor will only accelerate but never slow down.", "http://docs.unity3d.com/ScriptReference/JointMotor-freeSpin.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("targetVelocity"),
                new DescriptorAttribute("Target Velocity", "The motor will apply a force up to force to achieve targetVelocity.", "http://docs.unity3d.com/ScriptReference/JointMotor-targetVelocity.html")));
        }
    }
}