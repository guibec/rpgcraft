using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(CharacterJoint), true)]
    public class CharacterJointEditor : JointEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();

            Type type = typeof(CharacterJoint);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("swingAxis"),
                new DescriptorAttribute("Swing Axis", "The secondary axis around which the joint can rotate.", "http://docs.unity3d.com/ScriptReference/CharacterJoint-swingAxis.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("lowTwistLimit"), new ExpandableAttribute(), 
                new DescriptorAttribute("Low Twist Limit", "The lower limit around the primary axis of the character joint.", "http://docs.unity3d.com/ScriptReference/CharacterJoint-lowTwistLimit.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("highTwistLimit"), new ExpandableAttribute(), 
                new DescriptorAttribute("High Twist Limit", "The upper limit around the primary axis of the character joint.", "http://docs.unity3d.com/ScriptReference/CharacterJoint-highTwistLimit.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("swing1Limit"), new ExpandableAttribute(), 
                new DescriptorAttribute("Swing Limit 1", "The limit around the primary axis of the character joint.", "http://docs.unity3d.com/ScriptReference/CharacterJoint-swing1Limit.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("swing2Limit"), new ExpandableAttribute(), 
                new DescriptorAttribute("Swing Limit 2", "The limit around the primary axis of the character joint.", "http://docs.unity3d.com/ScriptReference/CharacterJoint-swing2Limit.html")));
        }
    }
}
