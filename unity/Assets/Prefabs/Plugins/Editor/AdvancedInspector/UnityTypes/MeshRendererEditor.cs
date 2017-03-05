using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(MeshRenderer), true)]
    public class MeshRendererEditor : RendererEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
        }
    }
}
