using System;
using System.Linq;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(TerrainCollider), true)]
    public class TerrainColliderEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(TerrainCollider);
            if (Instances == null || Instances.Length == 0)
                return;

            SerializedObject so = new SerializedObject(Instances.Cast<UnityEngine.Object>().ToArray());

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("material"),
                new DescriptorAttribute("Material", "The material used by the collider.", "http://docs.unity3d.com/ScriptReference/Collider-material.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("terrainData"),
                new DescriptorAttribute("Terrain Data", "The terrain that stores the heightmap.", "http://docs.unity3d.com/ScriptReference/TerrainCollider-terrainData.html")));
            Fields.Add(new InspectorField(type, Instances, so.FindProperty("m_EnableTreeColliders"),
                new DescriptorAttribute("Enable Tree Colliders", "", "")));
        }
    }
}
