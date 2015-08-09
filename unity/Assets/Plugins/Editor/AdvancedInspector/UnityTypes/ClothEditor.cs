using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

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

            fields.Add(new InspectorField(type, Instances, type.GetProperty("bendingStiffness"),
                new DescriptorAttribute("Bending Stiffness", "Is the collider a trigger?", "http://docs.unity3d.com/ScriptReference/Collider-isTrigger.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("stretchingStiffness"),
                new DescriptorAttribute("Stretching Stiffness", "", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("damping"),
                new DescriptorAttribute("Damping", "", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("thickness"),
                new DescriptorAttribute("Thickness", "", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("useGravity"),
                new DescriptorAttribute("Use Gravity", "", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("selfCollision"),
                new DescriptorAttribute("Self Collision", "", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("externalAcceleration"),
                new DescriptorAttribute("External Acceleration", "", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("randomAcceleration"),
                new DescriptorAttribute("Random Acceleration", "", "")));
        }
    }
}