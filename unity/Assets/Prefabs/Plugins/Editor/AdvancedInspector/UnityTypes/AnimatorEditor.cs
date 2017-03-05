using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(Animator), true)]
    public class AnimatorEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Animator);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("runtimeAnimatorController"), new HelpAttribute(new HelpAttribute.HelpDelegate(HelpController)),
                new DescriptorAttribute("Controller", "The runtime representation of AnimatorController that controls the Animator.", "http://docs.unity3d.com/ScriptReference/Animator-runtimeAnimatorController.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("avatar"),
                new DescriptorAttribute("Avatar", "Gets/Sets the current Avatar.", "http://docs.unity3d.com/ScriptReference/Animator-avatar.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("applyRootMotion"),
                new DescriptorAttribute("Apply Root Motion", "Should root motion be applied?", "http://docs.unity3d.com/ScriptReference/Animator-applyRootMotion.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("updateMode"),
                new DescriptorAttribute("Update Mode", "Specifies the update mode of the Animator.", "http://docs.unity3d.com/ScriptReference/Animator-updateMode.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("cullingMode"),
                new DescriptorAttribute("Culling Mode", "Controls culling of this Animator component.", "http://docs.unity3d.com/ScriptReference/Animator-cullingMode.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("fireEvents"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Fire Events", "If true, the animation track fires their event.", "")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("logWarnings"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Log Warnings", "Log Animator internal warnings.", "")));

            InspectorField transform = new InspectorField("Transform");

            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("speed"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Speed", "The playback speed of the Animator. 1 is normal playback speed.", "http://docs.unity3d.com/ScriptReference/Animator-speed.html")));
#if UNITY_5_0 || UNITY_5_1 || UNITY_5_2 || UNITY_5_3
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("bodyPosition"), new InspectAttribute(InspectorLevel.Advanced), new ReadOnlyAttribute(),
                new DescriptorAttribute("Body Position", "The position of the body center of mass.", "http://docs.unity3d.com/ScriptReference/Animator-bodyPosition.html")));
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("rootPosition"), new InspectAttribute(InspectorLevel.Advanced), new ReadOnlyAttribute(),
                new DescriptorAttribute("Root Position", "The root position, the position of the game object.", "http://docs.unity3d.com/ScriptReference/Animator-rootPosition.html")));
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("bodyRotation"), new InspectAttribute(InspectorLevel.Advanced), new ReadOnlyAttribute(),
                new DescriptorAttribute("Body Rotation", "The rotation of the body center of mass.", "http://docs.unity3d.com/ScriptReference/Animator-bodyRotation.html")));
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("rootRotation"), new InspectAttribute(InspectorLevel.Advanced), new ReadOnlyAttribute(),
                new DescriptorAttribute("Root Rotation", "The root rotation, the rotation of the game object.", "http://docs.unity3d.com/ScriptReference/Animator-rootRotation.html")));
#endif

            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("targetPosition"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Target Position", "Returns the position of the target specified by SetTarget(AvatarTarget targetIndex, float targetNormalizedTime)).", "http://docs.unity3d.com/ScriptReference/Animator-targetPosition.html")));
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("targetRotation"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Target Rotation", "Returns the rotation of the target specified by SetTarget(AvatarTarget targetIndex, float targetNormalizedTime)).", "http://docs.unity3d.com/ScriptReference/Animator-targetRotation.html")));

            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("deltaPosition"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Delta Position", "Gets the avatar delta position for the last evaluated frame.", "http://docs.unity3d.com/ScriptReference/Animator-deltaPosition.html")));
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("deltaRotation"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Delta Rotation", "Gets the avatar delta rotation for the last evaluated frame.", "http://docs.unity3d.com/ScriptReference/Animator-deltaRotation.html")));

            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("pivotPosition"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Pivot Position", "Get the current position of the pivot.", "http://docs.unity3d.com/ScriptReference/Animator-pivotPosition.html")));
            transform.Fields.Add(new InspectorField(type, Instances, type.GetProperty("pivotWeight"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Pivot Weight", "Gets the pivot weight.", "http://docs.unity3d.com/ScriptReference/Animator-pivotWeight.html")));

            InspectorField feet = new InspectorField("Feet");

            feet.Fields.Add(new InspectorField(type, Instances, type.GetProperty("layersAffectMassCenter"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Layers Affect Mass Center", "", "")));
            feet.Fields.Add(new InspectorField(type, Instances, type.GetProperty("feetPivotActive"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Feet Pivot", "Blends pivot point between body center of mass and feet pivot. At 0%, the blending point is body center of mass. At 100%, the blending point is feet pivot.", "http://docs.unity3d.com/ScriptReference/Animator-feetPivotActive.html")));
            feet.Fields.Add(new InspectorField(type, Instances, type.GetProperty("stabilizeFeet"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Stabilize Feet", "Automatic stabilization of feet during transition and blending.", "http://docs.unity3d.com/ScriptReference/Animator-stabilizeFeet.html")));
            feet.Fields.Add(new InspectorField(type, Instances, type.GetProperty("leftFeetBottomHeight"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Left Feet Bottom Height", "Get left foot bottom height.", "http://docs.unity3d.com/ScriptReference/Animator-leftFeetBottomHeight.html")));
            feet.Fields.Add(new InspectorField(type, Instances, type.GetProperty("rightFeetBottomHeight"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Right Feet Bottom Height", "Get right foot bottom height.", "http://docs.unity3d.com/ScriptReference/Animator-rightFeetBottomHeight.html")));

            InspectorField debug = new InspectorField("Debug");

            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("gravityWeight"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Gravity Weight", "The current gravity weight based on current animations that are played.", "http://docs.unity3d.com/ScriptReference/Animator-gravityWeight.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("hasRootMotion"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Has Root Motion", "Returns true if the current rig has root motion.", "http://docs.unity3d.com/ScriptReference/Animator-hasRootMotion.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("hasTransformHierarchy"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Has Tranform Hierarchy", "Returns true if the object has a transform hierarchy.", "http://docs.unity3d.com/ScriptReference/Animator-hasTransformHierarchy.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("humanScale"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Human Scale", "Returns the scale of the current Avatar for a humanoid rig, (1 by default if the rig is generic).", "http://docs.unity3d.com/ScriptReference/Animator-humanScale.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("isHuman"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Is Human", "Returns true if the current rig is humanoid, false if it is generic.", "http://docs.unity3d.com/ScriptReference/Animator-isHuman.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("isMatchingTarget"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Is Matching Target", "If automatic matching is active.", "http://docs.unity3d.com/ScriptReference/Animator-isMatchingTarget.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("isOptimizable"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Is Optimizable", "Returns true if the current rig is optimizable with AnimatorUtility.OptimizeTransformHierarchy.", "http://docs.unity3d.com/ScriptReference/Animator-isOptimizable.html")));

#if UNITY_5_0 || UNITY_5_1 || UNITY_5_2 || UNITY_5_3
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("layerCount"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Layer Count", "The AnimatorController layer count.", "http://docs.unity3d.com/ScriptReference/Animator-layerCount.html")));
#endif

            Fields.Add(transform);
            Fields.Add(feet);
            Fields.Add(debug);
        }

        private HelpItem HelpController()
        {
            foreach (object instance in Instances)
            {
                Animator animator = instance as Animator;

                if (animator == null)
                    continue;

                if (animator.runtimeAnimatorController == null)
                    return new HelpItem(HelpType.Error, "The Animator requires a controller to work.");
            }

            return null;
        }
    }
}
