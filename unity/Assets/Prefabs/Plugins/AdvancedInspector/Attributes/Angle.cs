using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Turns a float/int into a spinning knob.
    /// Because... Fancy.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class AngleAttribute : Attribute, IListAttribute
    {
        private float snap = -1;

        /// <summary>
        /// Makes the control snap to the multiple of that value
        /// Default; -1. Negative values turn this behaviour off.
        /// </summary>
        public float Snap
        {
            get { return snap; }
            set { snap = value; }
        }

        public AngleAttribute() { }

        /// <summary>
        /// If snap is -1, the snap is disable.
        /// Snap makes the wheel "stick" to multiple of a fixed value.
        /// </summary>
        public AngleAttribute(float snap)
        {
            this.snap = snap;
        }
    }
}