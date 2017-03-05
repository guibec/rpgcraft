using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Prevent a nested object from having to be unfolded.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class DisplayAsParentAttribute : Attribute
    {
        private bool hideParent = true;

        public bool HideParent
        {
            get { return hideParent; }
            set { hideParent = value; }
        }

        public DisplayAsParentAttribute() { }

        public DisplayAsParentAttribute(bool hideParent)
        {
            this.hideParent = hideParent;
        }
    }
}