using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Redefine if a field/property can be expanded or not.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property, Inherited = true)]
    public class ExpandableAttribute : Attribute, IListAttribute 
    {
        private bool expanded = false;

        /// <summary>
        /// Makes the item expanded by default.
        /// </summary>
        public bool Expanded
        {
            get { return expanded; }
            set { expanded = value; }
        }

        private bool expandable = true;

        /// <summary>
        /// Default true, can force a field to not be expandable. 
        /// </summary>
        public bool Expandable
        {
            get { return expandable; }
            set { expandable = value; }
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
    }
}