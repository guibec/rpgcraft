using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(Animation), true)]
    public class AnimationEditor : InspectorEditor
    {
        private static readonly int hash = "AnimationBoundsEditorHash".GetHashCode();
        private object boxEditor;
        private MethodInfo edit;
        private MethodInfo set;
        private Color handleColor = new Color(1, 1, 1, 0.6f);

        protected override void OnEnable()
        {
            base.OnEnable();

            Type box = TypeUtility.GetTypeByName("BoxEditor");
            boxEditor = Activator.CreateInstance(box, true, hash);
            edit = box.GetMethod("OnSceneGUI", new Type[] { typeof(Transform), typeof(Color), typeof(Vector3).MakeByRefType(), typeof(Vector3).MakeByRefType() });
            set = box.GetMethod("SetAlwaysDisplayHandles");
        }

        protected override void RefreshFields()
        {
            Type type = typeof(Animation);
            if (Instances == null || Instances.Length == 0)
                return;

            SerializedObject so = new SerializedObject(Instances.Cast<UnityEngine.Object>().ToArray());

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("clip"),
                new DescriptorAttribute("Animation", "The default animation.", "http://docs.unity3d.com/ScriptReference/Animation-clip.html")));
            Fields.Add(new InspectorField(type, Instances, so.FindProperty("m_Animations"),
                new DescriptorAttribute("Animations", "")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("playAutomatically"),
                new DescriptorAttribute("Play Automatically", "Should the default animation clip (Animation.clip) automatically start playing on startup.", "http://docs.unity3d.com/ScriptReference/Animation-playAutomatically.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("animatePhysics"),
                new DescriptorAttribute("Animate Physic", "When turned on, animations will be executed in the physics loop. This is only useful in conjunction with kinematic rigidbodies.", "http://docs.unity3d.com/ScriptReference/Animation-animatePhysics.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("cullingType"),
                new DescriptorAttribute("Culling Type", "Controls culling of this Animation component.", "http://docs.unity3d.com/ScriptReference/Animation-cullingType.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("isPlaying"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Is Playing", "Are we playing any animations?", "http://docs.unity3d.com/ScriptReference/Animation-isPlaying.html")));

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("localBounds"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Bounds", "AABB of this Animation animation component in local space.", "http://docs.unity3d.com/ScriptReference/Animation-localBounds.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("wrapMode"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Wrap Mode", "How should time beyond the playback range of the clip be treated?", "http://docs.unity3d.com/ScriptReference/Animation-wrapMode.html")));

        }

        protected override void OnSceneGUI()
        {
            Animation animation = (Animation)target;

            if ((animation != null) && ((animation.cullingType == AnimationCullingType.BasedOnRenderers) || (animation.cullingType == AnimationCullingType.BasedOnRenderers)))
            {
                set.Invoke(boxEditor, new object[] { animation.cullingType == AnimationCullingType.BasedOnRenderers });
                Bounds localBounds = animation.localBounds;
                Vector3 center = localBounds.center;
                Vector3 size = localBounds.size;

                object[] arguments = new object[] { animation.transform, handleColor, center, size };

                if ((bool)edit.Invoke(boxEditor, arguments))
                {
                    center = (Vector3)arguments[2];
                    size = (Vector3)arguments[3];

                    Undo.RecordObject(animation, "Modified Animation bounds");
                    animation.localBounds = new Bounds(center, size);
                }
            }
        }
    }
}
