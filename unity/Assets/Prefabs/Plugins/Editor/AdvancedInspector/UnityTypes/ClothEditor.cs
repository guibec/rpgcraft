using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    //[CustomEditor(typeof(Cloth), true)]
    public class ClothEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Cloth);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("bendingStiffness"),
                new DescriptorAttribute("Bending Stiffness", "Is the collider a trigger?", "http://docs.unity3d.com/ScriptReference/Collider-isTrigger.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("stretchingStiffness"),
                new DescriptorAttribute("Stretching Stiffness", "", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("damping"),
                new DescriptorAttribute("Damping", "", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("thickness"),
                new DescriptorAttribute("Thickness", "", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("useGravity"),
                new DescriptorAttribute("Use Gravity", "", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("selfCollision"),
                new DescriptorAttribute("Self Collision", "", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("externalAcceleration"),
                new DescriptorAttribute("External Acceleration", "", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("randomAcceleration"),
                new DescriptorAttribute("Random Acceleration", "", "")));
        }
    }
}
