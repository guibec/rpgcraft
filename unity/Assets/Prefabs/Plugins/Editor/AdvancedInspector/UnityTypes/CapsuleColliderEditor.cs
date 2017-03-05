using System;
using System.Collections.Generic;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(CapsuleCollider), true)]
    public class CapsuleColliderEditor : ColliderEditor
    {
        private int ControlID;

        protected override void RefreshFields()
        {
            Type type = typeof(CapsuleCollider);

            base.RefreshFields();

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("center"),
                new DescriptorAttribute("Center", "The center of the capsule, measured in the object's local space.", "http://docs.unity3d.com/ScriptReference/CapsuleCollider-center.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("height"),
                new DescriptorAttribute("Height", "The height of the capsule meased in the object's local space.", "http://docs.unity3d.com/ScriptReference/CapsuleCollider-height.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("radius"),
                new DescriptorAttribute("Radius", "The radius of the sphere, measured in the object's local space.", "http://docs.unity3d.com/ScriptReference/CapsuleCollider-radius.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("direction"), new RestrictAttribute(new RestrictAttribute.RestrictDelegate(Direction)),
                new DescriptorAttribute("Direction", "The direction of the capsule.", "http://docs.unity3d.com/ScriptReference/CapsuleCollider-direction.html")));
        }

        private List<DescriptionPair> Direction()
        {
            List<DescriptionPair> list = new List<DescriptionPair>();
            foreach (AxisOrientation orientation in Enum.GetValues(typeof(AxisOrientation)))
                list.Add(new DescriptionPair((int)orientation, new Description(orientation.ToString(), "")));

            return list;
        }

        private Vector3 CapsuleExtends(CapsuleCollider target)
        {
            return new Vector3(target.radius, target.height, target.radius) + target.center;
        }

        private Matrix4x4 CapsuleOrientation(CapsuleCollider target)
        {
            if (target.direction == (int)AxisOrientation.YAxis)
                return Matrix4x4.TRS(target.transform.TransformPoint(target.center),
                    target.gameObject.transform.rotation, Vector3.one);
            else if (target.direction == (int)AxisOrientation.XAxis)
                return Matrix4x4.TRS(target.transform.TransformPoint(target.center),
                    target.transform.rotation * Quaternion.LookRotation(Vector3.up, Vector3.right), Vector3.one);
            else
                return Matrix4x4.TRS(target.transform.TransformPoint(target.center),
                    target.transform.rotation * Quaternion.LookRotation(Vector3.right, Vector3.forward), Vector3.one); 
        }

        protected override void OnSceneGUI()
        {
            base.OnSceneGUI();

            if (Event.current.type == EventType.Used)
                return;

            CapsuleCollider collider = (CapsuleCollider)target;

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

            Vector3 capsuleExtents = CapsuleExtends(collider);
            Matrix4x4 matrix = CapsuleOrientation(collider);

            float y = capsuleExtents.y - collider.center.y - 1;
            float x = capsuleExtents.x - collider.center.x;

            int hotControl = GUIUtility.hotControl;
            Vector3 localPos = Vector3.up * y;

            float value = SizeHandle(localPos, Vector3.up, matrix);
            if (!GUI.changed)
                value = SizeHandle(-localPos, Vector3.down, matrix);

            if (GUI.changed)
                collider.height += value / y / collider.height;

            value = SizeHandle(Vector3.left * x, Vector3.left, matrix);
            if (!GUI.changed)
                value = SizeHandle(-Vector3.left * x, -Vector3.left, matrix);

            if (!GUI.changed)
                value = SizeHandle(Vector3.forward * x, Vector3.forward, matrix);

            if (!GUI.changed)
                value = SizeHandle(-Vector3.forward * x, -Vector3.forward, matrix);

            if (GUI.changed)
                collider.radius += value / Mathf.Max(capsuleExtents.z / collider.radius, capsuleExtents.x / collider.radius);

            if (hotControl != GUIUtility.hotControl && GUIUtility.hotControl != 0)
                ControlID = GUIUtility.hotControl;

            if (GUI.changed)
            {
                Undo.RecordObject(collider, "Edited Capsule Collider");
                collider.radius = Mathf.Max(collider.radius, 0.001f);
                collider.height = Mathf.Max(collider.height, 0.001f);
            }

            Handles.color = color;
            GUI.enabled = enabled;
        }

        private float SizeHandle(Vector3 localPos, Vector3 localPullDir, Matrix4x4 matrix)
        {
            bool changed = GUI.changed;
            GUI.changed = false;

            Vector3 rhs = matrix.MultiplyVector(localPullDir);
            Vector3 position = matrix.MultiplyPoint(localPos);
            float handleSize = HandleUtility.GetHandleSize(position);

            Color color = Handles.color;
            float angle = Mathf.Cos(0.7853982f);

            float dot;
            if (Camera.current.orthographic)
                dot = Vector3.Dot(-Camera.current.transform.forward, rhs);
            else
                dot = Vector3.Dot((Camera.current.transform.position - position).normalized, rhs);

            if (dot < -angle)
                Handles.color = new Color(Handles.color.r, Handles.color.g, Handles.color.b, Handles.color.a * 0.2f);

            Vector3 point = Handles.Slider(position, rhs, handleSize * 0.03f, new Handles.DrawCapFunction(Handles.DotCap), 0f);

            float result = 0f;
            if (GUI.changed)
                result = HandleUtility.PointOnLineParameter(point, position, rhs);

            GUI.changed |= changed;
            Handles.color = color;

            return result;
        }
    }

    public enum AxisOrientation
    { 
        XAxis = 0,
        YAxis = 1,
        ZAxis = 2
    }
}
