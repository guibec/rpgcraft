using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEditor;
using UnityEngine;

namespace AdvancedInspector
{
    [CanEditMultipleObjects]
    [CustomEditor(typeof(FixedJoint), true)]
    public class FixedJointEditor : JointEditor
    {
        protected override void RefreshFields()
        {
            base.RefreshFields();
        }
    }
}
