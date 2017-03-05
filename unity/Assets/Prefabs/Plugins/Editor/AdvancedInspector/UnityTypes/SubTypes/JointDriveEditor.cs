using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(JointDrive), true)]
    public class JointDriveEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(JointDrive);

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("maximumForce"),
                new DescriptorAttribute("Maximum Force", "Amount of force applied to push the object toward the defined direction.", "http://docs.unity3d.com/ScriptReference/JointDrive-maximumForce.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("mode"),
                new DescriptorAttribute("Mode", "Whether the drive should attempt to reach position, velocity, both or nothing.", "http://docs.unity3d.com/ScriptReference/JointDrive-mode.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("positionDamper"),
                new DescriptorAttribute("Position Damper", "Resistance strength against the Position Spring. Only used if mode includes Position.", "http://docs.unity3d.com/ScriptReference/JointDrive-positionDamper.html")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("positionSpring"),
                new DescriptorAttribute("Position Spring", "Strength of a rubber-band pull toward the defined direction. Only used if mode includes Position.", "http://docs.unity3d.com/ScriptReference/JointDrive-positionSpring.html")));
        }
    }
}