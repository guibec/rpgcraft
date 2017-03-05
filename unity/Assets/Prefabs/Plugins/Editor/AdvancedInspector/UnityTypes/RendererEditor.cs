using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;

namespace AdvancedInspector
{
    public abstract class RendererEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(Renderer);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("shadowCastingMode"),
                new DescriptorAttribute("Cast Shadows", "Does this object cast shadows?", "http://docs.unity3d.com/ScriptReference/Renderer-shadowCastingMode.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("receiveShadows"),
                new DescriptorAttribute("Receive Shadows", "Does this object receive shadows?", "http://docs.unity3d.com/ScriptReference/Renderer-receiveShadows.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMaterials"),
                new DescriptorAttribute("Materials", "All the shared materials of this object.", "http://docs.unity3d.com/ScriptReference/Renderer-sharedMaterials.html")));
#if UNITY_5_0 || UNITY_5_1 || UNITY_5_2 || UNITY_5_3
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("useLightProbes"),
                new DescriptorAttribute("Use Light Probes", "Use light probes for this Renderer.", "http://docs.unity3d.com/ScriptReference/Renderer-useLightProbes.html")));
#else
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("lightProbeUsage"),
                new DescriptorAttribute("Light Probes", "The light probe interpolation type.", "http://docs.unity3d.com/540/Documentation/ScriptReference/Renderer-lightProbeUsage.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("lightProbeProxyVolumeOverride"),
                new DescriptorAttribute("Proxy Volume Override", "If set, the Renderer will use the Light Probe Proxy Volume component attached to the source game object.", "http://docs.unity3d.com/540/Documentation/ScriptReference/Renderer-lightProbeProxyVolumeOverride.html"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsUsingLightProbeOverride))));
#endif
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("probeAnchor"), new InspectAttribute(new InspectAttribute.InspectDelegate(IsUsingLightProbes)),
                new DescriptorAttribute("Anchor Override", "If set, Renderer will use this Transform's position to find the interpolated light probe.", "http://docs.unity3d.com/ScriptReference/Renderer-lightProbeAnchor.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("reflectionProbeUsage"),
                new DescriptorAttribute("Reflection Probes", "Should reflection probes be used for this Renderer?", "http://docs.unity3d.com/ScriptReference/Renderer-reflectionProbeUsage.html")));

            Type editor = typeof(RendererEditor);
            Fields.Add(new InspectorField(editor, new UnityEngine.Object[] { this }, editor.GetProperty("ReflectionProbes", BindingFlags.NonPublic | BindingFlags.Instance),
                       new InspectAttribute(new InspectAttribute.InspectDelegate(IsUsingReflectionProbes)), 
                       new CollectionAttribute(0, false), new ReadOnlyAttribute(), new DisplayAsParentAttribute()));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("isPartOfStaticBatch"),
                new DescriptorAttribute("Static Batched", "Has this renderer been statically batched with any other renderers?", "http://docs.unity3d.com/ScriptReference/Renderer-isPartOfStaticBatch.html"), new InspectAttribute(InspectorLevel.Debug)));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("isVisible"),
                new DescriptorAttribute("Is Visible", "Is this renderer visible in any camera? (Read Only)", "http://docs.unity3d.com/ScriptReference/Renderer-isVisible.html"), new InspectAttribute(InspectorLevel.Debug)));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("lightmapIndex"),
                new DescriptorAttribute("Lightmap Index", "The index of the lightmap applied to this renderer.", "http://docs.unity3d.com/ScriptReference/Renderer-lightmapIndex.html"), new InspectAttribute(InspectorLevel.Debug)));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sortingLayerID"),
                new DescriptorAttribute("Sorting Layer ID", "ID of the Renderer's sorting layer.", "http://docs.unity3d.com/ScriptReference/Renderer-sortingLayerID.html"), new InspectAttribute(InspectorLevel.Debug)));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sortingLayerName"),
                new DescriptorAttribute("Sorting Layer Name", "Name of the Renderer's sorting layer.", "http://docs.unity3d.com/ScriptReference/Renderer-sortingLayerName.html"), new InspectAttribute(InspectorLevel.Debug)));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sortingOrder"),
                new DescriptorAttribute("Sorting Order", "Renderer's order within a sorting layer.", "http://docs.unity3d.com/ScriptReference/Renderer-sortingOrder.html"), new InspectAttribute(InspectorLevel.Debug)));
        }

        private bool IsUsingLightProbes()
        {
#if UNITY_5_0 || UNITY_5_1 || UNITY_5_2 || UNITY_5_3
            for (int i = 0; i < Instances.Length; i++)
                if (!((Renderer)Instances[i]).useLightProbes)
                    return false;
#else
            for (int i = 0; i < Instances.Length; i++)
                if (((Renderer)Instances[i]).lightProbeUsage == LightProbeUsage.Off)
                    return false;
#endif

            return true;
        }

        private bool IsUsingLightProbeOverride()
        {
#if UNITY_5_0 || UNITY_5_1 || UNITY_5_2 || UNITY_5_3
            return false;
#else
            for (int i = 0; i < Instances.Length; i++)
                if (((Renderer)Instances[i]).lightProbeUsage != LightProbeUsage.UseProxyVolume)
                    return false;

            return true;
#endif
        }

        private bool IsUsingReflectionProbes()
        {
            for (int i = 0; i < Instances.Length; i++)
                if (((Renderer)Instances[i]).reflectionProbeUsage == ReflectionProbeUsage.Off)
                    return false;

            return true;
        }

        private ReflectionProbeBlendInfo[] ReflectionProbes
        {
            get
            {
                if (Instances.Length > 1)
                    return new ReflectionProbeBlendInfo[0];

                Renderer renderer = Instances[0] as Renderer;
                List<ReflectionProbeBlendInfo> blends = new List<ReflectionProbeBlendInfo>();
                renderer.GetClosestReflectionProbes(blends);
                return blends.ToArray();
            }
        }
    }
}
