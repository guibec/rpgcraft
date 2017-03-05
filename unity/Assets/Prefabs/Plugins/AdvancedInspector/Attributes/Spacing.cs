using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Add a space after the current fields.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method)]
    public class SpacingAttribute : Attribute, ISpacing
    {
        private int before = 0;

        /// <summary>
        /// Size of the space to add before the item.
        /// Default is 0.
        /// </summary>
        public int Before
        {
            get { return before; }
            set { before = value; }
        }

        private int after = 0;

        /// <summary>
        /// Size of the space to add after the item.
        /// Default is 1.
        /// </summary>
        public int After
        {
            get { return after; }
            set { after = value; }
        }

        public SpacingAttribute() { }

        public SpacingAttribute(int after)
        {
            this.after = after;
        }

        public SpacingAttribute(int before, int after)
        {
            this.after = after;
            this.before = before;
        }

        #region ISpacing Implementation
        public int GetAfter(object[] instances, object[] values)
        {
            return after;
        }

        public int GetBefore(object[] instances, object[] values)
        {
            return before;
        }
        #endregion
    }
}