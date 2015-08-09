using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

#if UNITY_4_0 || UNITY_4_1 || UNITY_4_2 || UNITY_4_3 || UNITY_4_4 || UNITY_4_5 || UNITY_4_6
namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(ClothRenderer), true)]
    public class ClothRendererEditor : RendererEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(ClothRenderer);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("pauseWhenNotVisible"),
                new DescriptorAttribute("Pause When Not Visible", "Should the cloth simulation be paused when the ClothRenderer is not visible?", "http://docs.unity3d.com/ScriptReference/ClothRenderer-pauseWhenNotVisible.html")));
        }
    }
}
#endif