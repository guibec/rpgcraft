using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

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

            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("extremumSlip"),
                new DescriptorAttribute("Extremum Slip", "")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("extremumValue"),
                new DescriptorAttribute("Extremum Value", "")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("asymptoteSlip"),
                new DescriptorAttribute("Asymptote Slip", "")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("asymptoteValue"),
                new DescriptorAttribute("Asymptote Value", "")));
            fields.Add(new InspectorField(parent, type, Instances, type.GetProperty("stiffness"),
                new DescriptorAttribute("Stiffness", "")));
        }
    }
}