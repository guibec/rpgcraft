using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(MeshCollider), true)]
    public class MeshColliderEditor : ColliderEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(MeshCollider);

            base.RefreshFields();

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("convex"),
                new DescriptorAttribute("Convex", "Use a convex collider from the mesh.", "http://docs.unity3d.com/ScriptReference/MeshCollider-convex.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMesh"), new HelpAttribute(new HelpAttribute.HelpDelegate(HelpMesh)),
                new DescriptorAttribute("Mesh", "The mesh object used for collision detection.", "http://docs.unity3d.com/ScriptReference/MeshCollider-sharedMesh.html")));
        }

        private HelpItem HelpMesh()
        {
            foreach (object instance in Instances)
            {
                MeshCollider collider = instance as MeshCollider;

                if (collider == null)
                    continue;

                if (collider.sharedMesh == null)
                    return new HelpItem(HelpType.Error, "The Mesh Collider requires a Mesh to work.");
            }

            return null;
        }
    }
}
