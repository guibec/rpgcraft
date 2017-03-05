using System;
using UnityEngine;

namespace AdvancedInspector
{
    public class FieldAttributeExampleDrawer : FieldEditor
    {
        public override Type[] EditedTypes
        {
            get { return new Type[] { typeof(FieldAttributeExample) }; }
        }

        public override void Draw(FieldAttribute attribute, InspectorField field)
        {
            object value = field.GetValue();
            if (value == null)
                return;

            GUILayout.Label(value.ToString());
        }
    }
}
