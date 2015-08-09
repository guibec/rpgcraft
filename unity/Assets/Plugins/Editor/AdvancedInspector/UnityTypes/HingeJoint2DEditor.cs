using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(HingeJoint2D), true)]
    public class HingeJoint2DEditor : AnchoredJoint2DEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(HingeJoint2D);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("useLimits"), 
                new DescriptorAttribute("Use Limit", "Should limits be placed on the range of rotation?", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-useLimits.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("limits"), new ExpandableAttribute(), new InspectAttribute(new InspectAttribute.InspectDelegate(UsesLimits)),
                new DescriptorAttribute("Limits", "Limit of angular rotation on the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-limits.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("useMotor"),
                new DescriptorAttribute("Use Motor", "Should the joint be rotated automatically by a motor torque?", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-useMotor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("motor"), new ExpandableAttribute(), new InspectAttribute(new InspectAttribute.InspectDelegate(UsesMotor)),
                new DescriptorAttribute("Motor", "Parameters for the motor force applied to the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-motor.html")));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("jointAngle"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Angle", "The current joint angle with respect to the reference angle.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-jointAngle.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("jointSpeed"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Speed", "The current joint speed.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-jointSpeed.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("limitState"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Limit State", "Gets the state of the joint limit.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-limitState.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("referenceAngle"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Reference Angle", "The angle referenced between the two bodies used as the constraint for the joint.", "http://docs.unity3d.com/ScriptReference/HingeJoint2D-referenceAngle.html")));
        }

        private bool UsesLimits()
        {
            foreach (object instance in Instances)
            {
                HingeJoint2D hinge = instance as HingeJoint2D;
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
                HingeJoint2D hinge = instance as HingeJoint2D;
                if (hinge == null)
                    continue;

                if (hinge.useMotor)
                    return true;
            }

            return false;
        }
    }
}