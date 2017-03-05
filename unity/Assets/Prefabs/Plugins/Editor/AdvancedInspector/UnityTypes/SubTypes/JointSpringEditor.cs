using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(JointSpring), true)]
    public class JointSpringEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(JointSpring);

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetField("damper"),
                new DescriptorAttribute("Damper", "The damper force uses to dampen the spring.", "http://docs.unity3d.com/ScriptReference/JointSpring-damper.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetField("spring"),
                new DescriptorAttribute("Spring", "The spring forces used to reach the target position.", "http://docs.unity3d.com/ScriptReference/JointSpring-spring.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetField("targetPosition"),
                new DescriptorAttribute("Target Position", "The target position the joint attempts to reach.", "http://docs.unity3d.com/ScriptReference/JointSpring-targetPosition.html")));
        }
    }
}