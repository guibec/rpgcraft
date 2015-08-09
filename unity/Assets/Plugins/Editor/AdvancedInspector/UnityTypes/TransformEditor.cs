using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(Transform), true)]
    public class TransformEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Transform);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("localPosition"),
                new DescriptorAttribute("Position", "Position of the transform relative to the parent transform.", "http://docs.unity3d.com/ScriptReference/Transform-localPosition.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("localEulerAngles"),
                new DescriptorAttribute("Rotation", "The rotation of the transform relative to the parent transform's rotation.", "http://docs.unity3d.com/ScriptReference/Transform-localRotation.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("localScale"),
                new DescriptorAttribute("Scale", "The scale of the transform relative to the parent.", "http://docs.unity3d.com/ScriptReference/Transform-localScale.html")));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("position"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("World Position", "The position of the transform in world space.", "http://docs.unity3d.com/ScriptReference/Transform-position.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("rotation"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("World Rotation", "The rotation of the transform in world space stored as a Quaternion.", "http://docs.unity3d.com/ScriptReference/Transform-rotation.html")));
        }
    }
}
