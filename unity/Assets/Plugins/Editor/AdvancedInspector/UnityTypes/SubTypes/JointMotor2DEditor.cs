using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(JointMotor2D), true)]
    public class JointMotor2DEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(JointMotor2D);

            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("maxMotorTorque"),
                new DescriptorAttribute("Maximum Motor Torque", "The maximum force that can be applied to the Rigidbody2D at the joint to attain the target speed.", "http://docs.unity3d.com/ScriptReference/JointMotor2D-maxMotorTorque.html")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("motorSpeed"),
                new DescriptorAttribute("Motor Speed", "The desired speed for the Rigidbody2D to reach as it moves with the joint.", "http://docs.unity3d.com/ScriptReference/JointMotor2D-motorSpeed.html")));
        }
    }
}