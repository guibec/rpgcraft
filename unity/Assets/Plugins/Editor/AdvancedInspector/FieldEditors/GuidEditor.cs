using UnityEngine;
using UnityEditor;
using System;
using System.Collections;

namespace AdvancedInspector
{
    public class GuidEditor : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(Guid) }; }
        }

        public override void Draw(InspectorField field, GUIStyle style)
        {
            // GetValue returns null if multi-selection and different values
            Guid id = field.GetValue<Guid>();
            if (id == Guid.Empty)
                GUILayout.Label("GUID: --------------");
            else
                GUILayout.Label("GUID: " + id.ToString());
        }
    }
}