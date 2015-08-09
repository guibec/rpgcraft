using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    //[CustomEditor(typeof(Renderer), true)]
    public class RendererEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Renderer);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("shadowCastingMode"),
                new DescriptorAttribute("Cast Shadows", "Does this object cast shadows?", "http://docs.unity3d.com/ScriptReference/Renderer-shadowCastingMode.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("receiveShadows"),
                new DescriptorAttribute("Receive Shadows", "Does this object receive shadows?", "http://docs.unity3d.com/ScriptReference/Renderer-receiveShadows.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("useLightProbes"),
                new DescriptorAttribute("Use Light Probes", "Use light probes for this Renderer.", "http://docs.unity3d.com/ScriptReference/Renderer-useLightProbes.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("probeAnchor"), new InspectAttribute(new InspectAttribute.InspectDelegate(IsUsingLightProbe)),
                new DescriptorAttribute("Probes Anchor", "If set, Renderer will use this Transform's position to find the interpolated light probe.", "http://docs.unity3d.com/ScriptReference/Renderer-lightProbeAnchor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMaterials"),
                new DescriptorAttribute("Materials", "All the shared materials of this object.", "http://docs.unity3d.com/ScriptReference/Renderer-sharedMaterials.html")));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("isPartOfStaticBatch"),
                new DescriptorAttribute("Static Batched", "Has this renderer been statically batched with any other renderers?", "http://docs.unity3d.com/ScriptReference/Renderer-isPartOfStaticBatch.html"), new InspectAttribute(InspectorLevel.Debug)));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("isVisible"),
                new DescriptorAttribute("Is Visible", "Is this renderer visible in any camera? (Read Only)", "http://docs.unity3d.com/ScriptReference/Renderer-isVisible.html"), new InspectAttribute(InspectorLevel.Debug)));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("lightmapIndex"),
                new DescriptorAttribute("Lightmap Index", "The index of the lightmap applied to this renderer.", "http://docs.unity3d.com/ScriptReference/Renderer-lightmapIndex.html"), new InspectAttribute(InspectorLevel.Debug)));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("sortingLayerID"),
                new DescriptorAttribute("Sorting Layer ID", "ID of the Renderer's sorting layer.", "http://docs.unity3d.com/ScriptReference/Renderer-sortingLayerID.html"), new InspectAttribute(InspectorLevel.Debug)));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("sortingLayerName"),
                new DescriptorAttribute("Sorting Layer Name", "Name of the Renderer's sorting layer.", "http://docs.unity3d.com/ScriptReference/Renderer-sortingLayerName.html"), new InspectAttribute(InspectorLevel.Debug)));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("sortingOrder"),
                new DescriptorAttribute("Sorting Order", "Renderer's order within a sorting layer.", "http://docs.unity3d.com/ScriptReference/Renderer-sortingOrder.html"), new InspectAttribute(InspectorLevel.Debug)));
        }

        private bool IsUsingLightProbe()
        {
            for (int i = 0; i < Instances.Length; i++)
                if (!((Renderer)Instances[i]).useLightProbes)
                    return false;

            return true;
        }
    }
}
