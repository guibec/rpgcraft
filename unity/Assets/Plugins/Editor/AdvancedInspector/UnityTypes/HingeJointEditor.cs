using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(HingeJoint), true)]
    public class HingeJointEditor : JointEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(HingeJoint);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("useLimits"), 
                new DescriptorAttribute("Use Limits", "Limit of angular rotation on the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-limits.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("limits"), new ExpandableAttribute(), new InspectAttribute(new InspectAttribute.InspectDelegate(UsesLimits)),
                new DescriptorAttribute("Limits", "Should the joint be rotated automatically by a motor torque?", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-useMotor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("useMotor"), 
                new DescriptorAttribute("Use Motor", "Parameters for the motor force applied to the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-motor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("motor"), new ExpandableAttribute(), new InspectAttribute(new InspectAttribute.InspectDelegate(UsesMotor)),
                new DescriptorAttribute("Motor", "Parameters for the motor force applied to the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-motor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("useSpring"), 
                new DescriptorAttribute("Use Spring", "Parameters for the motor force applied to the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-motor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("spring"), new ExpandableAttribute(), new InspectAttribute(new InspectAttribute.InspectDelegate(UsesSpring)),
                new DescriptorAttribute("Spring", "Parameters for the motor force applied to the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-motor.html")));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("angle"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Angle", "Should limits be placed on the range of rotation?", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-useLimits.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("velocity"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Velocity", "The current joint angle with respect to the reference angle.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-jointAngle.html")));
        }

        private bool UsesLimits()
        {
            foreach (object instance in Instances)
            {
                HingeJoint hinge = instance as HingeJoint;
                if (hinge == null)
                    continue;

                if (hinge.useLimits)
                    return true;
            }

            return false;
        }

        private bool UsesMotor()
        {
            foreach (object instance in Instances)
            {
                HingeJoint hinge = instance as HingeJoint;
                if (hinge == null)
                    continue;

                if (hinge.useMotor)
                    return true;
            }

            return false;
        }

        private bool UsesSpring()
        {
            foreach (object instance in Instances)
            {
                HingeJoint hinge = instance as HingeJoint;
                if (hinge == null)
                    continue;

                if (hinge.useSpring)
                    return true;
            }

            return false;
        }
    }
}