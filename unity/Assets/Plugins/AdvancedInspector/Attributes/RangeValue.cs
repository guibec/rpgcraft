using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Similar to Unity's "Range" attribute but for the Advanced Inspector.
    /// However, Unity's version is flagged to be "Field Only", while this one can be placed on Properties.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class RangeValueAttribute : Attribute, IListAttribute
    {
        private float min;

        /// <summary>
        /// Min value, the current value cannot go below that.
        /// </summary>
        public float Min
        {
            get { return min; }
            set { min = value; }
        }

        private float max;

        /// <summary>
        /// Max value, the current value cannot go above that.
        /// </summary>
        public float Max
        {
            get { return max; }
            set { max = value; }
        }

        public RangeValueAttribute(float min, float max)
        {
            this.min = min;
            this.max = max;
        }
    }
}