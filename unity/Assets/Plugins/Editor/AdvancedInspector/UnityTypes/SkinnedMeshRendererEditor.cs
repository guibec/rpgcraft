using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(SkinnedMeshRenderer), true)]
    public class SkinnedMeshRendererEditor : RendererEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(SkinnedMeshRenderer);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("quality"),
                new DescriptorAttribute("Quality", "The maximum number of bones affecting a single vertex.", "http://docs.unity3d.com/ScriptReference/SkinnedMeshRenderer-quality.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("updateWhenOffscreen"),
                new DescriptorAttribute("Update Off Screen", "If enabled, the Skinned Mesh will be updated when offscreen. If disabled, this also disables updating animations.", "http://docs.unity3d.com/ScriptReference/SkinnedMeshRenderer-updateWhenOffscreen.html")));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMesh"),
                new DescriptorAttribute("Mesh", "The mesh used for skinning.", "http://docs.unity3d.com/ScriptReference/SkinnedMeshRenderer-sharedMesh.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("rootBone"), 
                new DescriptorAttribute("Root", "The root boot of the skeletton hierarchy.")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("bones"),
                new CollectionAttribute(-1), new DescriptorAttribute("Bones", "The bones used to skin the mesh.", "http://docs.unity3d.com/ScriptReference/SkinnedMeshRenderer-bones.html"), new InspectAttribute(InspectorLevel.Advanced)));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("localBounds"),
                new DescriptorAttribute("Bounds", "AABB of this Skinned Mesh in its local space.", "http://docs.unity3d.com/ScriptReference/SkinnedMeshRenderer-localBounds.html"), new InspectAttribute(InspectorLevel.Advanced)));
        }
    }
}