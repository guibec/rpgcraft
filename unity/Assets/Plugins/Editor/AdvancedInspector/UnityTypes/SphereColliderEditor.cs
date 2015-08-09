using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(SphereCollider), true)]
    public class SphereColliderEditor : ColliderEditor
    {
        private int ControlID = -1;

        protected override void RefreshFields()
        {
            Type type = typeof(SphereCollider);

            base.RefreshFields();

            fields.Add(new InspectorField(type, Instances, type.GetProperty("center"),
                new DescriptorAttribute("Center", "The center of the sphere, measured in the object's local space.", "http://docs.unity3d.com/ScriptReference/SphereCollider-center.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("radius"),
                new DescriptorAttribute("Radius", "The radius of the sphere, measured in the object's local space.", "http://docs.unity3d.com/ScriptReference/SphereCollider-radius.html")));
        }

        protected override void OnSceneGUI()
        {
            base.OnSceneGUI();

            if (Event.current.type == EventType.Used)
                return;

            SphereCollider collider = (SphereCollider)target;
            Color color = Handles.color;

            if (collider.enabled)
                Handles.color = ColliderHandleColor;
            else
                Handles.color = ColliderHandleColorDisabled;

            bool enabled = GUI.enabled;
            if (!Event.current.shift && GUIUtility.hotControl != ControlID)
            {
                GUI.enabled = false;
                Handles.color = new Color(0f, 0f, 0f, 0.001f);
            }

            Vector3 lossyScale = collider.transform.lossyScale;
            float x = Mathf.Abs(lossyScale.x);

            float scale = Mathf.Max(Mathf.Max(x, Mathf.Abs(lossyScale.y)), Mathf.Abs(lossyScale.z));
            float radius = Mathf.Max(Mathf.Abs(scale * collider.radius), 0.001f);

            Vector3 position = collider.transform.TransformPoint(collider.center);
            Quaternion rotation = collider.transform.rotation;

            int hotControl = GUIUtility.hotControl;
            float value = Handles.RadiusHandle(rotation, position, radius, true);

            if (GUI.changed)
            {
                Undo.RecordObject(collider, "Edited Sphere Collider");
                collider.radius = value / scale;
            }

            if (hotControl != GUIUtility.hotControl && GUIUtility.hotControl != 0)
                ControlID = GUIUtility.hotControl;

            Handles.color = color;
            GUI.enabled = enabled;
        }
    }
}