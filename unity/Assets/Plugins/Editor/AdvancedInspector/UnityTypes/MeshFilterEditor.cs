using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(MeshFilter), true)]
    public class MeshFilterEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(MeshFilter);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMesh"), new ExpandableAttribute(),
                new DescriptorAttribute("Mesh", "Returns the shared mesh of the mesh filter.", "http://docs.unity3d.com/ScriptReference/MeshFilter-sharedMesh.html")));
        }
    }
}