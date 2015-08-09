using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    public class AnchoredJoint2DEditor : Joint2DEditor
    {
        private const float snapDistance = 0.13f;
        private AnchoredJoint2D anchorJoint2D;

        protected override void RefreshFields()
        {
            base.RefreshFields();
            Type type = typeof(AnchoredJoint2D);

            fields.Add(new InspectorField(type, Instances, type.GetProperty("anchor"),
                new DescriptorAttribute("Anchor", "The joint's anchor point on the object that has the joint component.", "http://docs.unity3d.com/ScriptReference/AnchoredJoint2D-anchor.html")));
            fields.Add(new InspectorField(type, Instances, type.GetProperty("connectedAnchor"),
                new DescriptorAttribute("Connected Anchor", "The joint's anchor point on the second object (ie, the one which doesn't have the joint component).", "http://docs.unity3d.com/ScriptReference/AnchoredJoint2D-connectedAnchor.html")));
        }

        protected override void OnSceneGUI()
        {
            anchorJoint2D = (AnchoredJoint2D)target;
            Vector3 v1 = TransformPoint(anchorJoint2D.transform, anchorJoint2D.anchor);
            Vector3 v2 = anchorJoint2D.connectedAnchor;

            if (anchorJoint2D.connectedBody)
                v2 = TransformPoint(anchorJoint2D.connectedBody.transform, v2);

            Vector3 v3 = v1 + (v2 - v1).normalized * HandleUtility.GetHandleSize(v1) * 0.1f;
            Handles.color = Color.green;
            Handles.DrawAAPolyLine(new Vector3[] { v3, v2 });

            if (HandleAnchor(ref v2, true))
            {
                v2 = SnapToSprites(v2);
                v2 = SnapToPoint(v2, v1, 0.13f);
                if (anchorJoint2D.connectedBody)
                    v2 = InverseTransformPoint(anchorJoint2D.connectedBody.transform, v2);

                Undo.RecordObject(anchorJoint2D, "Move Connected Anchor");
                anchorJoint2D.connectedAnchor = v2;
            }

            if (HandleAnchor(ref v1, false))
            {
                v1 = SnapToSprites(v1);
                v1 = SnapToPoint(v1, v2, 0.13f);

                Undo.RecordObject(anchorJoint2D, "Move Anchor");
                anchorJoint2D.anchor = InverseTransformPoint(anchorJoint2D.transform, v1);
            }
        }

        private Vector3 SnapToSprites(Vector3 position)
        {
            SpriteRenderer component = anchorJoint2D.GetComponent<SpriteRenderer>();
            position = SnapToSprite(component, position, 0.13f);

            if (anchorJoint2D.connectedBody)
            {
                component = anchorJoint2D.connectedBody.GetComponent<SpriteRenderer>();
                position = SnapToSprite(component, position, 0.13f);
            }

            return position;
        }
    }
}