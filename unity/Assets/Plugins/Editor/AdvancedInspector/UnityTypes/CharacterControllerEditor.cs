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
    [CustomEditor(typeof(CharacterController), true)]
    public class CharacterControllerEditor : InspectorEditor
    {
        private int controlID = -1;

        private Color colliderHandleColor = new Color(145f, 244f, 139f, 210f) / 255f;
        private Color colliderHandleColorDisabled = new Color(84f, 200f, 77f, 140f) / 255f;

        protected override void RefreshFields()
        {
            Type type = typeof(CharacterController);
            if (Instances == null || Instances.Length == 0)
                return;

            SerializedObject so = new SerializedObject(Instances.Cast<UnityEngine.Object>().ToArray());

            fields.Add(new InspectorField(type, Instances, type.GetProperty("slopeLimit"),
                new DescriptorAttribute("Slope Limit", "The character controllers slope limit in degrees.", "http://docs.unity3d.com/ScriptReference/CharacterController-slopeLimit.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("stepOffset"),
                new DescriptorAttribute("Step Offset", "The character controllers step offset in meters.", "http://docs.unity3d.com/ScriptReference/CharacterController-stepOffset.html")));
            fields.Add(new InspectorField(type, Instances, so.FindProperty("m_SkinWidth"),
                new DescriptorAttribute("Skin Width", "The thickness of the interpenetration of this capsule.", "")));
            fields.Add(new InspectorField(type, Instances, so.FindProperty("m_MinMoveDistance"),
                new DescriptorAttribute("Min Move Distance", "The smallest distance required for the character to move.", "")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("center"),
                new DescriptorAttribute("Center", "The center of the character's capsule relative to the transform's position.", "http://docs.unity3d.com/ScriptReference/CharacterController-center.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("radius"),
                new DescriptorAttribute("Radius", "The radius of the character's capsule.", "http://docs.unity3d.com/ScriptReference/CharacterController-radius.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("height"),
                new DescriptorAttribute("Height", "The height of the character's capsule.", "http://docs.unity3d.com/ScriptReference/CharacterController-height.html")));

            fields.Add(new InspectorField(type, Instances, type.GetProperty("detectCollisions"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Detect Collisions", "Determines whether other rigidbodies or character controllers collide with this character controller (by default this is always enabled).", "http://docs.unity3d.com/ScriptReference/CharacterController-detectCollisions.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("isGrounded"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Is Grounded", "Was the CharacterController touching the ground during the last move?", "http://docs.unity3d.com/ScriptReference/CharacterController-isGrounded.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("collisionFlags"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Collision Flags", "What part of the capsule collided with the environment during the last CharacterController.Move call.", "http://docs.unity3d.com/ScriptReference/CharacterController-collisionFlags.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("velocity"), new InspectAttribute(InspectorLevel.Advanced),
                new DescriptorAttribute("Velocity", "The current relative velocity of the Character (see notes).", "http://docs.unity3d.com/ScriptReference/CharacterController-velocity.html")));
        }

        protected override void OnSceneGUI()
        {
            bool flag = GUIUtility.hotControl == controlID;

            CharacterController controller = (CharacterController)target;
            Color color = Handles.color;

            if (controller.enabled)
                Handles.color = colliderHandleColor;
            else
                Handles.color = colliderHandleColorDisabled;

            bool enabled = GUI.enabled;
            if (!Event.current.shift && !flag)
            {
                GUI.enabled = false;
                Handles.color = new Color(1f, 0f, 0f, 0.001f);
            }

            float height = controller.height * controller.transform.lossyScale.y;
            float radius = controller.radius * Mathf.Max(controller.transform.lossyScale.x, controller.transform.lossyScale.z);

            height = Mathf.Max(height, radius * 2f);

            Matrix4x4 matrix = Matrix4x4.TRS(controller.transform.TransformPoint(controller.center), Quaternion.identity, Vector3.one);
            Vector3 localPos = (Vector3.up * height) * 0.5f;

            float size = SizeHandle(localPos, Vector3.up, matrix, true);
            if (!GUI.changed)
                size = SizeHandle(-localPos, Vector3.down, matrix, true);

            size = SizeHandle((Vector3)(Vector3.left * radius), Vector3.left, matrix, true);
            if (!GUI.changed)
                size = SizeHandle((Vector3)(-Vector3.left * radius), -Vector3.left, matrix, true);

            if (!GUI.changed)
                size = SizeHandle((Vector3)(Vector3.forward * radius), Vector3.forward, matrix, true);

            if (!GUI.changed)
                size = SizeHandle((Vector3)(-Vector3.forward * radius), -Vector3.forward, matrix, true);

            if (GUI.changed)
            {
                Undo.RecordObject(controller, "Character Controller Resize");

                controller.radius += size / (radius / controller.radius);
                controller.height += size / (height / controller.height);

                controller.radius = Mathf.Max(controller.radius, 1E-05f);
                controller.height = Mathf.Max(controller.height, 1E-05f);
            }

            int hotControl = GUIUtility.hotControl;
            if ((hotControl != GUIUtility.hotControl) && (GUIUtility.hotControl != 0))
                controlID = GUIUtility.hotControl;

            Handles.color = color;
            GUI.enabled = enabled;
        }

        private static float SizeHandle(Vector3 localPos, Vector3 localPullDir, Matrix4x4 matrix, bool isEdgeHandle)
        {
            Vector3 rhs = matrix.MultiplyVector(localPullDir);
            Vector3 position = matrix.MultiplyPoint(localPos);
            float handleSize = HandleUtility.GetHandleSize(position);
            bool changed = GUI.changed;
            GUI.changed = false;
            Color color = Handles.color;

            float edge = 0f;
            if (isEdgeHandle)
                edge = Mathf.Cos(0.7853982f);

            float dot;

            if (Camera.current.orthographic)
                dot = Vector3.Dot(-Camera.current.transform.forward, rhs);
            else
            {
                Vector3 vector4 = Camera.current.transform.position - position;
                dot = Vector3.Dot(vector4.normalized, rhs);
            }

            if (dot < -edge)
                Handles.color = new Color(Handles.color.r, Handles.color.g, Handles.color.b, Handles.color.a * 0.2f);

            Vector3 point = Handles.Slider(position, rhs, handleSize * 0.03f, new Handles.DrawCapFunction(Handles.DotCap), 0f);

            float distance = 0f;
            if (GUI.changed)
                distance = HandleUtility.PointOnLineParameter(point, position, rhs);

            GUI.changed |= changed;
            Handles.color = color;
            return distance;
        }
    }
}