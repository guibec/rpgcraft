using System;

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

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("force"),
                new DescriptorAttribute("Force", "The force applied to the rigidbody every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-force.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("relativeForce"),
                new DescriptorAttribute("Relative Force", "The force - relative to the rigid bodies coordinate system - applied every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-relativeForce.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("torque"),
                new DescriptorAttribute("Torque", "The torque applied to the rigidbody every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-torque.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("relativeTorque"),
                new DescriptorAttribute("Relative Torque", "The torque - relative to the rigid bodies coordinate system - applied every frame.", "http://docs.unity3d.com/ScriptReference/ConstantForce-relativeTorque.html")));
        }
    }
}
