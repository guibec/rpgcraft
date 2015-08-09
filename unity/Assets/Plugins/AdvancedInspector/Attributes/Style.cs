using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Allow to change the style of an field item.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field | AttributeTargets.Method)]
    public class StyleAttribute : Attribute
    {
        private string style = "";

        /// <summary>
        /// Name of the style to use.
        /// Must be findable by GUI.skin.Find()
        /// </summary>
        public string Style
        {
            get { return style; }
            set { style = value; }
        }

        private bool label = true;

        /// <summary>
        /// Force or prevent the field's label from being displayed.
        /// </summary>
        public bool Label
        {
            get { return label; }
            set { label = value; }
        }

        public StyleAttribute(string style)
            : this(style, true) { }

        public StyleAttribute(string style, bool label)
        {
            this.style = style;
            this.label = label;
        }
    }
}