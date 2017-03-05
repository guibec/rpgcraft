using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Similar to a PropertyAttribute, except it can be placed on an inspected property or method.
    /// It is also not serialization-bound.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method)]
    public abstract class FieldAttribute : Attribute
    {
        private int order = 0;

        /// <summary>
        /// In case of many FieldAttribute, the order they are rendered.
        /// </summary>
        public int Order
        {
            get { return order; }
            set { order = value; }
        }
    }
}
