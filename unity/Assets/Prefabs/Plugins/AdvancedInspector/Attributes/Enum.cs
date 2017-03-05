using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Controls how an enum is handled and displayed.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class EnumAttribute : Attribute, IListAttribute
    {
        private bool masked = false;

        /// <summary>
        /// Turns a normal enum into a bitfield.
        /// Careful, your enum should be properly setup to accepted bitfield input.
        /// </summary>
        public bool Masked
        {
            get { return masked; }
            set { masked = value; }
        }

        private EnumDisplay display = EnumDisplay.DropDown;

        /// <summary>
        /// Forces an enum to be displayed differently.
        /// </summary>
        public EnumDisplay Display
        {
            get { return display; }
            set { display = value; }
        }

        private int maxItemsPerRow = 6;

        /// <summary>
        /// When display is using Button or Checkbox, this is the maximum number of button displayed per rows before creating a new one.
        /// </summary>
        public int MaxItemsPerRow
        {
            get { return maxItemsPerRow; }
            set { maxItemsPerRow = value; }
        }

        public EnumAttribute(bool masked) 
        {
            this.masked = masked;
        }

        public EnumAttribute(EnumDisplay display)
        {
            this.display = display;
        }

        public EnumAttribute(bool masked, EnumDisplay display)
        {
            this.masked = masked;
            this.display = display;
        }
    }
}