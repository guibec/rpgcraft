using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(Rigidbody), true)]
    public class RigidbodyEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Rigidbody);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("mass"), 
                new DescriptorAttribute("Mass", "The mass of the rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-mass.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("drag"), 
                new DescriptorAttribute("Drag", "The drag of the object", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-drag.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("angularDrag"), 
                new DescriptorAttribute("Angular Drag", "The angular drag of the object", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-angularDrag.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("useGravity"), 
                new DescriptorAttribute("Use Gravity", "Is the object affected by world gravity", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-useGravity.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("isKinematic"), 
                new DescriptorAttribute("Is Kinematic", "If true, physic does not drive this object, used for animation", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-isKinematic.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("interpolation"), 
                new DescriptorAttribute("Interpolation", "Interpolation allows you to smooth out the effect of running physics at a fixed frame rate", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-interpolation.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("collisionDetectionMode"), 
                new DescriptorAttribute("Collision Detection", "How collision are detected", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-collisionDetectionMode.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("constraints"), new EnumAttribute(true),
                new DescriptorAttribute("Constraints", "Controls which degrees of freedom are allowed for the simulation of this Rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-constraints.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("centerOfMass"), new InspectAttribute(InspectorLevel.Advanced), 
                new DescriptorAttribute("Center of Mass", "In essence, the pivot around which the object spins", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-centerOfMass.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("freezeRotation"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Freeze Rotation", "Controls whether physics will change the rotation of the object", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-freezeRotation.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("inertiaTensor"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Inertia Tensor", "The diagonal inertia tensor of mass relative to the center of mass", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-inertiaTensor.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("inertiaTensorRotation"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Inertia Tensor Rotation", "The rotation of the inertia tensor", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-inertiaTensorRotation.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("maxAngularVelocity"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Max Angular Velocity", "The maximimum angular velocity of the rigidbody. (Default 7) range { 0, infinity }", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-maxAngularVelocity.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sleepThreshold"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Sleep Threshold", "The mass-normalized energy threshold, below which objects start going to sleep.", "http://docs.unity3d.com/ScriptReference/Rigidbody-sleepThreshold.html")));

#if UNITY_5_0 || UNITY_5_1 || UNITY_5_2 || UNITY_5_3
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("solverIterationCount"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Solver Iteration Count", "Allows you to override the solver iteration count per rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-solverIterationCount.html")));
#else
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("solverIterations"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Solver Iteration Count", "Allows you to override the solver iteration count per rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-solverIterationCount.html")));
#endif

            InspectorField debug = new InspectorField("Debug");
            Fields.Add(debug);

            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("position"), new ReadOnlyAttribute(), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Position", "The position of the rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-position.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("rotation"), new ReadOnlyAttribute(), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Rotation", "The rotation of the rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-rotation.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("velocity"), new ReadOnlyAttribute(), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Velocity", "The velocity of the rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-velocity.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("angularVelocity"), new ReadOnlyAttribute(), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Angular Velocity", "The rotation velocity of the rigidbody", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-angularVelocity.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("worldCenterOfMass"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("World Center of Mass", "The center of mass of the rigidbody in world space", "https://docs.unity3d.com/Documentation/ScriptReference/Rigidbody-worldCenterOfMass.html")));
        }
    }
}
