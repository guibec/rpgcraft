using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    //Under construction. Unity's editor has multiple internal method and classes used for this editor.
    /*[CanEditMultipleObjects]
    //[CustomEditor(typeof(SpriteRenderer), true)]
    public class SpriteRendererEditor : RendererEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(SpriteRenderer);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("subMeshCount"), 
                new DescriptorAttribute("Sub Mesh Count", "The number of submesh this mesh has.")));
        }

        private void IsMaterialUsingFixedFunction(out bool vertex, out bool fragment)
        {
            vertex = false;
            fragment = false;
            Material sharedMaterial = (base.target as SpriteRenderer).sharedMaterial;
            if (sharedMaterial != null)
            {
                vertex = ShaderUtil.GetVertexModel(sharedMaterial.shader) == ShaderUtil.ShaderModel.None;
                fragment = ShaderUtil.GetFragmentModel(sharedMaterial.shader) == ShaderUtil.ShaderModel.None;
            }
        }
    }*/
}