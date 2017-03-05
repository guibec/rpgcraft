using System;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(WheelFrictionCurve), true)]
    public class WheelFrictionCurveEditor : InspectorEditor
    {
        protected override void RefreshFields()
        {
            Type type = typeof(WheelFrictionCurve);

            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("extremumSlip"),
                new DescriptorAttribute("Extremum Slip", "")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("extremumValue"),
                new DescriptorAttribute("Extremum Value", "")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("asymptoteSlip"),
                new DescriptorAttribute("Asymptote Slip", "")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("asymptoteValue"),
                new DescriptorAttribute("Asymptote Value", "")));
            Fields.Add(new InspectorField(Parent, type, Instances, type.GetProperty("stiffness"),
                new DescriptorAttribute("Stiffness", "")));
        }
    }
}