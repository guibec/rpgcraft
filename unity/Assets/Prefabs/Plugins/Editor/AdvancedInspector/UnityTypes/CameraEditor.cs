using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(Camera), true)]
    public class CameraEditor : InspectorEditor
    {
        private Rect[] rects;
        private Camera camera;

        private SceneView view = null;

        public Vector2 screenshotResolution = new Vector2();

        protected override void RefreshFields()
        {
            Type type = typeof(Camera);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("clearFlags"),
                new DescriptorAttribute("Clear Flag", "How the camera clears the background.", "http://docs.unity3d.com/ScriptReference/Camera-clearFlags.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("backgroundColor"), new InspectAttribute(new InspectAttribute.InspectDelegate(ShowBackground)),
                new DescriptorAttribute("Background Color", "The color with which the screen will be cleared.", "http://docs.unity3d.com/ScriptReference/Camera-backgroundColor.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("cullingMask"), new FieldEditorAttribute("LayerMaskEditor"),
                new DescriptorAttribute("Culling Mask", "This is used to render parts of the scene selectively.", "http://docs.unity3d.com/ScriptReference/Camera-cullingMask.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("orthographic"), new RestrictAttribute(new RestrictAttribute.RestrictDelegate(Projection)),
                new DescriptorAttribute("Orthographic", "Is the camera orthographic (true) or perspective (false)?", "http://docs.unity3d.com/ScriptReference/Camera-orthographic.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("orthographicSize"), new InspectAttribute(new InspectAttribute.InspectDelegate(IsOrthographic)),
                new DescriptorAttribute("Size", "Camera's half-size when in orthographic mode.", "http://docs.unity3d.com/ScriptReference/Camera-orthographicSize.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("fieldOfView"), new InspectAttribute(new InspectAttribute.InspectDelegate(IsFieldOfView)),
                new RangeValueAttribute(1, 179), new DescriptorAttribute("Field Of View", "The field of view of the camera in degrees.", "http://docs.unity3d.com/ScriptReference/Camera-fieldOfView.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("nearClipPlane"),
                new DescriptorAttribute("Near Clip", "The near clipping plane distance.", "http://docs.unity3d.com/ScriptReference/Camera-nearClipPlane.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("farClipPlane"),
                new DescriptorAttribute("Far Clip", "The far clipping plane distance.", "http://docs.unity3d.com/ScriptReference/Camera-farClipPlane.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("rect"),
                new DescriptorAttribute("Viewport Rect", "Where on the screen is the camera rendered in normalized coordinates.", "http://docs.unity3d.com/ScriptReference/Camera-rect.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("depth"),
                new DescriptorAttribute("Depth", "Camera's depth in the camera rendering order.", "http://docs.unity3d.com/ScriptReference/Camera-depth.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("renderingPath"), new HelpAttribute(new HelpAttribute.HelpDelegate(RenderingHelp)),
                new DescriptorAttribute("Rendering Path", "Rendering path.", "http://docs.unity3d.com/ScriptReference/Camera-renderingPath.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("targetTexture"),
                new DescriptorAttribute("Render Texture", "Destination render texture (Unity Pro only).", "http://docs.unity3d.com/ScriptReference/Camera-targetTexture.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("useOcclusionCulling"),
                new DescriptorAttribute("Occlusion Culling", "Whether or not the Camera will use occlusion culling during rendering.", "http://docs.unity3d.com/ScriptReference/Camera-useOcclusionCulling.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("hdr"),
                new DescriptorAttribute("HDR", "High dynamic range rendering.", "http://docs.unity3d.com/ScriptReference/Camera-hdr.html")));

            Fields.Add(new InspectorField(null, new UnityEngine.Object[] { this }, new object[] { this }, this.GetType().GetMethod("TakeScreenshot"),
                new Attribute[] { new InspectAttribute(InspectorLevel.Advanced) }));
            Fields.Add(new InspectorField(null, new UnityEngine.Object[] { this }, new object[] { this }, null, this.GetType().GetField("screenshotResolution"), false,
                new Attribute[] { new InspectAttribute(InspectorLevel.Advanced) }));

            // Debug
            InspectorField debug = new InspectorField("Debug");
            Fields.Add(debug);

            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("aspect"), new InspectAttribute(InspectorLevel.Debug), new ReadOnlyAttribute(),
                new DescriptorAttribute("Aspect Ratio", "The aspect ratio (width divided by height).", "http://docs.unity3d.com/ScriptReference/Camera-aspect.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("clearStencilAfterLightingPass"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Clear Stencil After Lighting", "Clear Stencil After Lighting Pass.")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("depthTextureMode"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Depth Texture Mode", "How and if camera generates a depth texture.", "http://docs.unity3d.com/ScriptReference/Camera-depthTextureMode.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("eventMask"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Event Mask", "Mask to select which layers can trigger events on the camera.", "http://docs.unity3d.com/ScriptReference/Camera-eventMask.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("layerCullDistances"), new InspectAttribute(InspectorLevel.Debug), new CollectionAttribute(0),
                new DescriptorAttribute("Layer Cull Distances", "Per-layer culling distances.", "http://docs.unity3d.com/ScriptReference/Camera-layerCullDistances.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("layerCullSpherical"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Layer Cull Spherical", "How to perform per-layer culling for a Camera.", "http://docs.unity3d.com/ScriptReference/Camera-layerCullSpherical.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("pixelRect"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Pixel Rect", "Where on the screen is the camera rendered in pixel coordinates.", "http://docs.unity3d.com/ScriptReference/Camera-pixelRect.html")));
            debug.Fields.Add(new InspectorField(type, Instances, type.GetProperty("transparencySortMode"), new InspectAttribute(InspectorLevel.Debug),
                new DescriptorAttribute("Transparency Sort Mode", "Transparent object sorting mode.", "http://docs.unity3d.com/ScriptReference/Camera-transparencySortMode.html")));

            if (camera == null)
            {
                camera = EditorUtility.CreateGameObjectWithHideFlags("Preview Camera", HideFlags.HideAndDontSave, new Type[] { typeof(Camera) }).GetComponent<Camera>();
                camera.enabled = false;
            }

            rects = new Rect[Instances.Length];

            for (int i = 0; i < Instances.Length; i++)
                rects[i] = new Rect(25, 25, 0, 0);
        }

        protected override void OnSceneGUI()
        {
            base.OnSceneGUI();

            if (Event.current.type == EventType.Used)
                return;

            if (view != SceneView.currentDrawingSceneView)
                InitPosition(SceneView.currentDrawingSceneView);

            Handles.BeginGUI();

            for (int i = 0; i < Instances.Length; i++)
            {
                Camera instance = Instances[i] as Camera;
                rects[i] = GUILayout.Window(i, rects[i], DrawWindow, instance.name + " Preview");
            }

            Handles.EndGUI();
        }

        public void TakeScreenshot()
        {
            Camera camera = target as Camera;
            if (camera == null)
                return;

            RenderTexture texture = RenderTexture.GetTemporary((int)screenshotResolution.x, (int)screenshotResolution.y);
            camera.targetTexture = texture;
            camera.Render();
            camera.targetTexture = null;
            RenderTexture.active = texture;

            Texture2D image = new Texture2D((int)screenshotResolution.x, (int)screenshotResolution.y);
            image.ReadPixels(new Rect(0, 0, (int)screenshotResolution.x, (int)screenshotResolution.y), 0, 0);
            image.Apply();

            RenderTexture.ReleaseTemporary(texture);
            RenderTexture.active = null;

            byte[] file = image.EncodeToPNG();

            int count = 1;
            string path;
            while (true)
            {
                path = Application.dataPath + "/Screenshot_" + count.ToString() + ".png";
                FileInfo info = new FileInfo(path);
                if (!info.Exists)
                    break;

                count++;
            }

            File.WriteAllBytes(path, file);
        }

        private void InitPosition(SceneView view)
        {
            this.view = view;

            int offset = 45;
            for (int i = 0; i < Instances.Length; i++)
            {
                Type gameView = TypeUtility.GetTypeByName("GameView");
                MethodInfo info = gameView.GetMethod("GetSizeOfMainGameView", BindingFlags.Static | BindingFlags.NonPublic);
                Vector2 camSize = (Vector2)info.Invoke(null, null);

                int width = (int)(camSize.x * 0.25f);
                int height = (int)(camSize.y * 0.25f);

                rects[i] = new Rect(view.position.width - width - 25, view.position.height - height - offset, 0, 0);
                offset += height + 35;
            }
        }

        private void DrawWindow(int i)
        {
            Rect rect = SceneView.currentDrawingSceneView.camera.pixelRect;

            Camera instance = Instances[i] as Camera;
            Type gameView = TypeUtility.GetTypeByName("GameView");
            MethodInfo info = gameView.GetMethod("GetSizeOfMainGameView", BindingFlags.Static | BindingFlags.NonPublic);
            Vector2 camSize = (Vector2)info.Invoke(null, null);

            int width = (int)(camSize.x * 0.25f);
            int height = (int)(camSize.y * 0.25f);

            camera.CopyFrom(instance);
            camera.pixelRect = new Rect(rects[i].x + 5, rect.height - rects[i].y - (height), width, height);
            camera.Render();
            GUI.DragWindow();
            GUI.Label(GUILayoutUtility.GetRect(width, height), "", GUIStyle.none);
        }

        private IList Projection()
        {
            List<object> list = new List<object>();
            list.Add(new DescriptionPair(false, new Description("Perspective", "")));
            list.Add(new DescriptionPair(true, new Description("Orthographic", "")));
            return list;
        }

        private bool ShowBackground()
        {
            foreach (object instance in Instances)
            {
                Camera camera = instance as Camera;

                if (camera == null)
                    continue;

                if (camera.clearFlags == CameraClearFlags.Depth || camera.clearFlags == CameraClearFlags.Nothing)
                    return false;
            }

            return true;
        }

        private bool IsOrthographic()
        {
            foreach (object instance in Instances)
            {
                Camera camera = instance as Camera;
                if (camera == null)
                    continue;

                if (!camera.orthographic)
                    return false;
            }

            return true;
        }

        private bool IsFieldOfView()
        {
            foreach (object instance in Instances)
            {
                Camera camera = instance as Camera;
                if (camera == null)
                    continue;

                if (camera.orthographic)
                    return false;
            }

            return true;
        }

        private HelpItem RenderingHelp()
        {
            foreach (object instance in Instances)
            {
                Camera camera = instance as Camera;
                if (camera == null)
                    return null;

                if (camera.renderingPath == RenderingPath.DeferredLighting && !UnityEditorInternal.InternalEditorUtility.HasPro())
                    return new HelpItem(HelpType.Warning, "Deferred lighting requires Unity Pro.");
            }

            return null;
        }
    }
}
