using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(WheelCollider), true)]
    public class WheelColliderEditor : ColliderEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(WheelCollider);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("mass"),
                new DescriptorAttribute("Mass", "The mass of the wheel. Must be larger than zero.", "http://docs.unity3d.com/ScriptReference/WheelCollider-mass.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("radius"),
                new DescriptorAttribute("Radius", "The radius of the wheel, measured in local space.", "http://docs.unity3d.com/ScriptReference/WheelCollider-radius.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("suspensionDistance"),
                new DescriptorAttribute("Suspension Distance", "Maximum extension distance of wheel suspension, measured in local space.", "http://docs.unity3d.com/ScriptReference/WheelCollider-suspensionDistance.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("center"),
                new DescriptorAttribute("Center", "The center of the wheel, measured in the object's local space.", "http://docs.unity3d.com/ScriptReference/WheelCollider-center.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("suspensionSpring"), new BypassAttribute(),
                new DescriptorAttribute("Suspension Spring", "The parameters of wheel's suspension. The suspension attempts to reach a target position.", "http://docs.unity3d.com/ScriptReference/WheelCollider-suspensionSpring.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("forwardFriction"),
                new DescriptorAttribute("Forward Friction", "Properties of tire friction in the direction the wheel is pointing in.", "http://docs.unity3d.com/ScriptReference/WheelCollider-forwardFriction.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sidewaysFriction"),
                new DescriptorAttribute("Sideways Friction", "Properties of tire friction in the sideways direction.", "http://docs.unity3d.com/ScriptReference/WheelCollider-sidewaysFriction.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("brakeTorque"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Brake Torque", "Brake torque. Must be positive.", "http://docs.unity3d.com/ScriptReference/WheelCollider-brakeTorque.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("motorTorque"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Motor Torque", "Motor torque on the wheel axle. Positive or negative depending on direction.", "http://docs.unity3d.com/ScriptReference/WheelCollider-motorTorque.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("steerAngle"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Steer Angle", "Steering angle in degrees, always around the local y-axis.", "http://docs.unity3d.com/ScriptReference/WheelCollider-steerAngle.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("isGrounded"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Is Grounded", "Indicates whether the wheel currently collides with something (Read Only).", "http://docs.unity3d.com/ScriptReference/WheelCollider-isGrounded.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("rpm"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("RPM", "Current wheel axle rotation speed, in rotations per minute (Read Only).", "http://docs.unity3d.com/ScriptReference/WheelCollider-rpm.html")));
        }
    }
}
