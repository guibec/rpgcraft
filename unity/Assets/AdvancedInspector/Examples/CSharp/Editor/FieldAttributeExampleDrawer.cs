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

        // Robin: This wouldn't compile. It's an example. Commented it out.
        //public override void Draw(AdvancedInspector.FieldAttribute attribute, InspectorField field)
        //{
        //    object value = field.GetValue();
        //    if (value == null)
        //        return;

        //    GUILayout.Label(value.ToString());
        //}
    }
}
