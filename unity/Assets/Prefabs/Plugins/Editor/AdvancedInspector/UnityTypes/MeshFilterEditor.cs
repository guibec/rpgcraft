using System;

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

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMesh"), new ExpandableAttribute(),
                new DescriptorAttribute("Mesh", "Returns the shared mesh of the mesh filter.", "http://docs.unity3d.com/ScriptReference/MeshFilter-sharedMesh.html")));
        }
    }
}
