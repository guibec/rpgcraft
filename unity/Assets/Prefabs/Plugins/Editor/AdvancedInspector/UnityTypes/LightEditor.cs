using System;
using System.Collections;
using System.Linq;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(Light), true)]
    public class LightEditor : InspectorEditor
    {
        private static Color disabledLightColor = new Color(0.5f, 0.45f, 0.2f, 0.5f);
        private static Color lightColor = new Color(0.95f, 0.95f, 0.5f, 0.5f);

        private SerializedProperty m_Lightmapping;

        protected override void RefreshFields()
        {
            Type type = typeof(Light);
            if (Instances == null || Instances.Length == 0)
                return;

            SerializedObject so = new SerializedObject(Instances.Cast<UnityEngine.Object>().ToArray());

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("type"),
                new DescriptorAttribute("Type", "The type of the light.", "http://docs.unity3d.com/ScriptReference/Light-type.html")));

            m_Lightmapping = so.FindProperty("m_Lightmapping");
            Fields.Add(new InspectorField(type, Instances, m_Lightmapping,
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsNotArea)),
                new RestrictAttribute(new RestrictAttribute.RestrictDelegate(Baking)),
                new DescriptorAttribute("Baking", "How the light is handled versus lightmaps.")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("range"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsPointOrSpot)),
                new DescriptorAttribute("Range", "The range of the light.", "http://docs.unity3d.com/ScriptReference/Light-range.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("spotAngle"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsSpot)),
                new DescriptorAttribute("Spot Angle", "The angle of the light's spotlight cone in degrees.", "http://docs.unity3d.com/ScriptReference/Light-spotAngle.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("areaSize"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsArea)),
                new DescriptorAttribute("Area Size", "The size of the area light. Editor only.", "http://docs.unity3d.com/ScriptReference/Light-areaSize.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("color"),
                new DescriptorAttribute("Color", "The color of the light.", "http://docs.unity3d.com/ScriptReference/Light-color.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("intensity"),
                new RangeValueAttribute(0f, 8f),
                new DescriptorAttribute("Intensity", "The Intensity of a light is multiplied with the Light color.", "http://docs.unity3d.com/ScriptReference/Light-intensity.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("bounceIntensity"),
                new RangeValueAttribute(0f, 8f), new HelpAttribute(new HelpAttribute.HelpDelegate(HelpBouncedGI)),
                new DescriptorAttribute("Bounce Intensity", "The multiplier that defines the strength of the bounce lighting.", "http://docs.unity3d.com/ScriptReference/Light-bounceIntensity.html")));



            //Acts like a group
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("shadows"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsNotArea)),
                new DescriptorAttribute("Shadow Type", "How this light casts shadows", "http://docs.unity3d.com/ScriptReference/Light-shadows.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("shadowStrength"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(HasShadow)),
                new DescriptorAttribute("Strength", "How this light casts shadows.", "http://docs.unity3d.com/ScriptReference/Light-shadowStrength.html")));
            Fields.Add(new InspectorField(type, Instances, so.FindProperty("m_Shadows.m_Resolution"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(HasShadow)),
                new DescriptorAttribute("Resolution", "The shadow's resolution.")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("shadowBias"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(HasShadow)),
                new DescriptorAttribute("Bias", "Shadow mapping bias.", "http://docs.unity3d.com/ScriptReference/Light-shadowBias.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("shadowNormalBias"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(HasShadow)),
                new DescriptorAttribute("Normal Bias", "Shadow mapping normal-based bias.", "http://docs.unity3d.com/ScriptReference/Light-shadowNormalBias.html")));

            Fields.Add(new InspectorField(type, Instances, so.FindProperty("m_DrawHalo"),
                new DescriptorAttribute("Draw Halo", "Draw a halo around the light. Now work with the Halo class.")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("cookie"),
                new DescriptorAttribute("Cookie", "The cookie texture projected by the light.", "http://docs.unity3d.com/ScriptReference/Light-cookie.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("cookieSize"),
                new InspectAttribute(new InspectAttribute.InspectDelegate(IsDirectional)),
                new DescriptorAttribute("Cookie Size", "The size of a directional light's cookie.", "http://docs.unity3d.com/ScriptReference/Light-cookieSize.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("flare"),
                new DescriptorAttribute("Flare", "The flare asset to use for this light.", "http://docs.unity3d.com/ScriptReference/Light-flare.html")));

            Fields.Add(new InspectorField(type, Instances, so.FindProperty("m_RenderMode"),
                new DescriptorAttribute("Render Mode", "The rendering path for the lights.")));

            Fields.Add(new InspectorField(type, Instances, so.FindProperty("m_CullingMask"),
                new HelpAttribute(new HelpAttribute.HelpDelegate(HelpSceneLighting)),
                new DescriptorAttribute("Culling Mask", "The object that are affected or ignored by the light.")));
        }

        private IList Baking()
        {
            return new DescriptionPair[] { new DescriptionPair(4, "Realtime", ""), new DescriptionPair(2, "Baked", ""), new DescriptionPair(1, "Mixed", "") };
        }

        public bool IsPointOrSpot()
        {
            if (IsPoint() || IsSpot())
                return true;

            return false;
        }

        public bool IsPoint()
        {
            return ((Light)Instances[0]).type == LightType.Point;
        }

        public bool IsSpot()
        {
            return ((Light)Instances[0]).type == LightType.Spot;
        }

        public bool IsDirectional()
        {
            return ((Light)Instances[0]).type == LightType.Directional;
        }

        public bool IsNotArea()
        {
            return !IsArea();
        }

        public bool IsArea()
        {
            return ((Light)Instances[0]).type == LightType.Area;
        }

        public bool HasShadow()
        {
            Light light = (Light)Instances[0];
            return IsNotArea() && (light.shadows == LightShadows.Hard || light.shadows == LightShadows.Soft);
        }

        public bool IsSoft()
        {
            return ((Light)Instances[0]).shadows == LightShadows.Soft;
        }

        public bool DoesAnyCameraUseDeferred()
        {
            Camera[] allCameras = Camera.allCameras;
            for (int i = 0; i < allCameras.Length; i++)
                if (allCameras[i].actualRenderingPath == RenderingPath.DeferredLighting)
                    return true;

            return false;
        }

        public HelpItem HelpBouncedGI()
        {
            if (((Light)Instances[0]).bounceIntensity > 0 && IsPointOrSpot() && m_Lightmapping.intValue != 2)
                return new HelpItem(HelpType.Warning, "Currently realtime indirect bounce light shadowing for spot and point lights is not supported.");

            return null;
        }

        public HelpItem HelpSceneLighting()
        {
            if (SceneView.currentDrawingSceneView != null && !SceneView.currentDrawingSceneView.m_SceneLighting)
                return new HelpItem(HelpType.Warning, "One of your scene views has lighting disable, please keep this in mind when editing lighting.");

            return null;
        }

        protected override void OnSceneGUI()
        {
            base.OnSceneGUI();

            if (Event.current.type == EventType.Used)
                return;

            Light light = (Light)target;
            Color color = Handles.color;

            if (light.enabled)
                Handles.color = lightColor;
            else
                Handles.color = disabledLightColor;

            float range = light.range;
            switch (light.type)
            {
                case LightType.Spot:
                {
                    Color color2 = Handles.color;
                    color2.a = Mathf.Clamp01(color.a * 2f);
                    Handles.color = color2;
                    Vector2 angleAndRange = new Vector2(light.spotAngle, light.range);
                    angleAndRange = ConeHandle(light.transform.rotation, light.transform.position, angleAndRange, 1f, 1f, true);
                    if (GUI.changed)
                    {
                        Undo.RecordObject(light, "Adjust Spot Light");
                        light.spotAngle = angleAndRange.x;
                        light.range = Mathf.Max(angleAndRange.y, 0.01f);
                    }

                    break;
                }

                case LightType.Point:
                {
                    range = Handles.RadiusHandle(Quaternion.identity, light.transform.position, range, true);
                    if (GUI.changed)
                    {
                        Undo.RecordObject(light, "Adjust Point Light");
                        light.range = range;
                    }

                    break;
                }

                case LightType.Area:
                {
                    EditorGUI.BeginChangeCheck();
                    Vector2 vector2 = RectHandles(light.transform.rotation, light.transform.position, light.areaSize);
                    if (EditorGUI.EndChangeCheck())
                    {
                        Undo.RecordObject(light, "Adjust Area Light");
                        light.areaSize = vector2;
                    }

                    break;
                }
            }
            Handles.color = color;
        }

        private Vector2 ConeHandle(Quaternion rotation, Vector3 position, Vector2 angleAndRange, float angleScale, float rangeScale, bool handlesOnly)
        {
            float x = angleAndRange.x;
            float y = angleAndRange.y;
            float r = y * rangeScale;
            
            Vector3 forward = rotation * Vector3.forward;
            Vector3 up = rotation * Vector3.up;
            Vector3 right = rotation * Vector3.right;

            bool changed = GUI.changed;
            GUI.changed = false;
            r = SizeSlider(position, forward, r);
            if (GUI.changed)
                y = Mathf.Max(0f, r / rangeScale);

            GUI.changed |= changed;
            changed = GUI.changed;
            GUI.changed = false;

            float angle = (r * Mathf.Tan((0.01745329f * x) / 2f)) * angleScale;
            angle = SizeSlider(position + (forward * r), up, angle);
            angle = SizeSlider(position + (forward * r), -up, angle);
            angle = SizeSlider(position + (forward * r), right, angle);
            angle = SizeSlider(position + (forward * r), -right, angle);

            if (GUI.changed)
                x = Mathf.Clamp((57.29578f * Mathf.Atan(angle / (r * angleScale))) * 2f, 0f, 179f);

            GUI.changed |= changed;
            if (!handlesOnly)
            {
                Handles.DrawLine(position, (position + (forward * r)) + (up * angle));
                Handles.DrawLine(position, (position + (forward * r)) - (up * angle));
                Handles.DrawLine(position, (position + (forward * r)) + (right * angle));
                Handles.DrawLine(position, (position + (forward * r)) - (right * angle));
                Handles.DrawWireDisc(position + r * forward, forward, angle);
            }

            return new Vector2(x, y);
        }

        private Vector2 RectHandles(Quaternion rotation, Vector3 position, Vector2 size)
        {
            Vector3 forward = rotation * Vector3.forward;
            Vector3 up = rotation * Vector3.up;
            Vector3 right = rotation * Vector3.right;

            float radiusX = 0.5f * size.x;
            float radiusY = 0.5f * size.y;

            Vector3 v1 = (position + (up * radiusY)) + (right * radiusX);
            Vector3 v2 = (position - (up * radiusY)) + (right * radiusX);
            Vector3 v3 = (position - (up * radiusY)) - (right * radiusX);
            Vector3 v4 = (position + (up * radiusY)) - (right * radiusX);

            Handles.DrawLine(v1, v2);
            Handles.DrawLine(v2, v3);
            Handles.DrawLine(v3, v4);
            Handles.DrawLine(v4, v1);

            Color color = Handles.color;
            color.a = Mathf.Clamp01(color.a * 2f);
            Handles.color = color;

            radiusY = SizeSlider(position, up, radiusY);
            radiusY = SizeSlider(position, -up, radiusY);
            radiusX = SizeSlider(position, right, radiusX);
            radiusX = SizeSlider(position, -right, radiusX);

            if (((Tools.current != Tool.Move) && (Tools.current != Tool.Scale)) || Tools.pivotRotation != PivotRotation.Local)
                Handles.DrawLine(position, position + forward);

            size.x = 2f * radiusX;
            size.y = 2f * radiusY;

            return size;
        }

        private float SizeSlider(Vector3 p, Vector3 direction, float radius)
        {
            Vector3 position = p + (direction * radius);
            float handleSize = HandleUtility.GetHandleSize(position);

            bool changed = GUI.changed;
            GUI.changed = false;

            position = Handles.Slider(position, direction, handleSize * 0.03f, new Handles.DrawCapFunction(Handles.DotCap), 0f);

            if (GUI.changed)
                radius = Vector3.Dot(position - p, direction);

            GUI.changed |= changed;
            return radius;
        }
    }
}
