using System;
using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Redefine if a field/property can be expanded or not.
    /// Note it has no effect on collection.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Class | AttributeTargets.Struct, Inherited = true)]
    public class ExpandableAttribute : Attribute, IExpandable, IListAttribute 
    {
        private bool expanded = false;

        /// <summary>
        /// Makes the item expanded by default.
        /// If a field is not expandable but expanded, the object is expanded but cannot be collapse.
        /// </summary>
        public bool Expanded
        {
            get { return expanded; }
            set { expanded = value; }
        }

        private bool expandable = true;

        /// <summary>
        /// Default true, can force a field to not be expandable. 
        /// If false, shows no inner fields.
        /// </summary>
        public bool Expandable
        {
            get { return expandable; }
            set { expandable = value; }
        }

        private bool alwaysExpanded = false;

        /// <summary>
        /// When true, this field is always expanded and cannot be collapsed.
        /// </summary>
        public bool AlwaysExpanded
        {
            get { return alwaysExpanded; }
            set { alwaysExpanded = value; }
        }

        public ExpandableAttribute() { }

        public ExpandableAttribute(bool expandable)
        {
            this.expandable = expandable;
        }

        public ExpandableAttribute(bool expandable, bool expanded)
        {
            this.expanded = expanded;
            this.expandable = expandable;
        }

        #region IExpandable Implementation
        public bool IsExpandable(object[] instances, object[] values)
        {
            return expandable;
        }

        public bool IsExpanded(object[] instances, object[] values)
        {
            return expanded;
        }

        public bool IsAlwaysExpanded(object[] instances, object[] values)
        {
            return alwaysExpanded;
        }
        #endregion
    }
}