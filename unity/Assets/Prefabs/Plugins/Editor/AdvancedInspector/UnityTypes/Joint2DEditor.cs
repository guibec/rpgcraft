using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    public class Joint2DEditor : InspectorEditor
    {
        public static GUIStyle anchor = null;
        public static GUIStyle anchorActive = null;
        public static GUIStyle connectedAnchor = null;
        public static GUIStyle connectedAnchorActive = null;

        protected override void RefreshFields()
        {
            Type type = typeof(Joint2D);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("enableCollision"),
                new DescriptorAttribute("Enable Collision", "Should rigid bodies connected with this joint collide?", "http://docs.unity3d.com/ScriptReference/Joint2D-enableCollision.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("connectedBody"),
                new DescriptorAttribute("Connected Body", "The Rigidbody2D object to which the other end of the joint is attached (ie, the object without the joint component).", "http://docs.unity3d.com/ScriptReference/Joint2D-connectedBody.html")));
        }

        protected bool HandleAnchor(ref Vector3 position, bool isConnectedAnchor)
        {
            if (anchor == null)
            {
                anchor = "U2D.pivotDot";
                anchorActive = "U2D.pivotDotActive";
                connectedAnchor = "U2D.dragDot";
                connectedAnchorActive = "U2D.dragDotActive";
            }

            Handles.DrawCapFunction drawFunc = (!isConnectedAnchor) ? new Handles.DrawCapFunction(AnchorCap) : new Handles.DrawCapFunction(ConnectedAnchorCap);
            int id = this.target.GetInstanceID() + ((!isConnectedAnchor) ? 0 : 1);
            EditorGUI.BeginChangeCheck();
            position = Handles.Slider2D(id, position, Vector3.back, Vector3.right, Vector3.up, 0f, drawFunc, Vector2.zero);
            return EditorGUI.EndChangeCheck();
        }

        public static void AnchorCap(int controlID, Vector3 position, Quaternion rotation, float size)
        {
            if (controlID == GUIUtility.keyboardControl)
                DrawCap(controlID, position, anchorActive);
            else
                DrawCap(controlID, position, anchor);
        }

        public static void ConnectedAnchorCap(int controlID, Vector3 position, Quaternion rotation, float size)
        {
            if (controlID == GUIUtility.keyboardControl)
                DrawCap(controlID, position, connectedAnchorActive);
            
            else
                DrawCap(controlID, position, connectedAnchor);
        }

        private static void DrawCap(int controlID, Vector3 position, GUIStyle guiStyle)
        {
            if (Event.current.type != EventType.Repaint)
            {
                return;
            }
            Handles.BeginGUI();
            position = HandleUtility.WorldToGUIPoint(position);
            float fixedWidth = guiStyle.fixedWidth;
            float fixedHeight = guiStyle.fixedHeight;
            Rect position2 = new Rect(position.x - fixedWidth / 2f, position.y - fixedHeight / 2f, fixedWidth, fixedHeight);
            guiStyle.Draw(position2, GUIContent.none, controlID);
            Handles.EndGUI();
        }

        public static void DrawAALine(Vector3 start, Vector3 end)
        {
            Handles.DrawAAPolyLine(new Vector3[] { start, end });
        }

        public static void DrawDistanceGizmo(Vector3 anchor, Vector3 connectedAnchor, float distance)
        {
            Vector3 normalized = (anchor - connectedAnchor).normalized;
            Vector3 vector = connectedAnchor + normalized * distance;
            Vector3 vector2 = Vector3.Cross(normalized, Vector3.forward);
            vector2 *= HandleUtility.GetHandleSize(connectedAnchor) * 0.16f;
            Handles.color = Color.green;
            DrawAALine(anchor, vector);
            DrawAALine(connectedAnchor + vector2, connectedAnchor - vector2);
            DrawAALine(vector + vector2, vector - vector2);
        }

        private static Matrix4x4 GetAnchorSpaceMatrix(Transform transform)
        {
            return Matrix4x4.TRS(transform.position, Quaternion.Euler(0f, 0f, transform.rotation.eulerAngles.z), transform.lossyScale);
        }

        protected static Vector3 TransformPoint(Transform transform, Vector3 position)
        {
            return GetAnchorSpaceMatrix(transform).MultiplyPoint(position);
        }

        protected static Vector3 InverseTransformPoint(Transform transform, Vector3 position)
        {
            return GetAnchorSpaceMatrix(transform).inverse.MultiplyPoint(position);
        }

        protected static Vector3 SnapToSprite(SpriteRenderer spriteRenderer, Vector3 position, float snapDistance)
        {
            if (spriteRenderer == null)
                return position;

            snapDistance = HandleUtility.GetHandleSize(position) * snapDistance;
            float num = spriteRenderer.sprite.bounds.size.x / 2f;
            float num2 = spriteRenderer.sprite.bounds.size.y / 2f;

            Vector2[] array = new Vector2[]
			{
				new Vector2(-num, -num2),
				new Vector2(0f, -num2),
				new Vector2(num, -num2),
				new Vector2(-num, 0f),
				new Vector2(0f, 0f),
				new Vector2(num, 0f),
				new Vector2(-num, num2),
				new Vector2(0f, num2),
				new Vector2(num, num2)
			};

            Vector2[] array2 = array;
            for (int i = 0; i < array2.Length; i++)
            {
                Vector2 v = array2[i];
                Vector3 vector = spriteRenderer.transform.TransformPoint(v);
                if (Vector2.Distance(position, vector) <= snapDistance)
                    return vector;
            }

            return position;
        }

        protected static Vector3 SnapToPoint(Vector3 position, Vector3 snapPosition, float snapDistance)
        {
            snapDistance = HandleUtility.GetHandleSize(position) * snapDistance;
            return (Vector3.Distance(position, snapPosition) > snapDistance) ? position : snapPosition;
        }

        protected static Vector2 RotateVector2(Vector2 direction, float angle)
        {
            float f = 0.0174532924f * -angle;
            float cos = Mathf.Cos(f);
            float sin = Mathf.Sin(f);
            float x = direction.x * cos - direction.y * sin;
            float y = direction.x * sin + direction.y * cos;
            return new Vector2(x, y);
        }
    }
}
