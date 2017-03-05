using System;
using UnityEngine;

namespace AdvancedInspector
{
    public abstract class ColliderEditor : InspectorEditor
    {
        public static Color ColliderHandleColor = new Color(0.57f, 0.96f, 0.54f, 0.82f);
        public static Color ColliderHandleColorDisabled = new Color(0.33f, 0.78f, 0.3f, 0.55f);

        protected override void RefreshFields()
        {
            Type type = typeof(Collider);

            Fields.Add(new InspectorField(type, Instances, type.GetProperty("isTrigger"),
                new DescriptorAttribute("Is Trigger", "Is the collider a trigger?", "http://docs.unity3d.com/ScriptReference/Collider-isTrigger.html")));
            Fields.Add(new InspectorField(type, Instances, type.GetProperty("sharedMaterial"), new InspectAttribute(new InspectAttribute.InspectDelegate(IsNotTrigger)),
                new DescriptorAttribute("Physic Material", "The shared physic material of this collider.", "http://docs.unity3d.com/ScriptReference/Collider-sharedMaterial.html")));
        }

        private bool IsNotTrigger()
        {
            for (int i = 0; i < Instances.Length; i++)
            {
                Collider collider = Instances[i] as Collider;

                if (!collider.isTrigger)
                    return true;
            }

            return false;
        }
    }
}