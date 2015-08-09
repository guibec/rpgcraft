using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    //[CustomEditor(typeof(Joint), true)]
    public class JointEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Joint);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("connectedBody"),
                new DescriptorAttribute("Connected Body", "A reference to another rigidbody this joint connects to.", "http://docs.unity3d.com/ScriptReference/Joint-connectedBody.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("anchor"),
                new DescriptorAttribute("Anchor", "The Position of the anchor around which the joints motion is constrained.", "http://docs.unity3d.com/ScriptReference/Joint-anchor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("axis"),
                new DescriptorAttribute("Axis", "The Direction of the axis around which the body is constrained.", "http://docs.unity3d.com/ScriptReference/Joint-axis.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("autoConfigureConnectedAnchor"),
                new DescriptorAttribute("Auto Configure Anchor", "Should the connectedAnchor be calculated automatically?", "http://docs.unity3d.com/ScriptReference/Joint-autoConfigureConnectedAnchor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("connectedAnchor"),
                new DescriptorAttribute("Connected Anchor", "Position of the anchor relative to the connected Rigidbody.", "http://docs.unity3d.com/ScriptReference/Joint-connectedAnchor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("breakForce"),
                new DescriptorAttribute("Break Force", "The force that needs to be applied for this joint to break.", "http://docs.unity3d.com/ScriptReference/Joint-breakForce.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("breakTorque"),
                new DescriptorAttribute("Break Torque", "The torque that needs to be applied for this joint to break.", "http://docs.unity3d.com/ScriptReference/Joint-breakTorque.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("enableCollision"),
                new DescriptorAttribute("Enable Collision", "Enable collision between bodies connected with the joint.", "http://docs.unity3d.com/ScriptReference/Joint-enableCollision.html")));
        }
    }
}