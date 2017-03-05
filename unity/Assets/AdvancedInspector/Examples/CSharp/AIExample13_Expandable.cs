using UnityEngine;
using System;
using System.Collections.Generic;

namespace AdvancedInspector
{
    public class AIExample13_Expandable : MonoBehaviour
    {
        // The expandable attribute is used to control if an item can not be expanded, or if it should be expanded by default.
        [Expandable(Expanded = true)]
        public ExpandableClass expandedField;

        // Preventing an object from being expandable is sometime useful when you want to display its inner content differently.
        [Expandable(Expandable = false)]
        public ExpandableClass notExpandableField;

        [Expandable(AlwaysExpanded = true)]
        public ExpandableClass alwaysExpanded;

        // This class is not expandable by default. 
        public NotExpandableClass notExpandableObject;

        // Class with the AdvancedInspector attribute are automaticly inlinable/expandable.
        [Serializable]
        public class ExpandableClass
        {
            public float myField;
        }

        // It is possible to prevent that like this;
        // This is useful when you build your own base type similar to how a float is displayed.
        // A good example is the Guid class.
        [Serializable, Expandable(false)]
        public class NotExpandableClass
        {
            public float myField;
        }
    }
}