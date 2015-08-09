using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Allow to groups inspector items.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field | AttributeTargets.Method)]
    public class ToolbarAttribute : Attribute
    {
        public const string ToolbarStyle = "Toolbar";

        private string name = "";

        /// <summary>
        /// Name of the toolbar, used to group items.
        /// If Label is true, the name is displayed.
        /// </summary>
        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        private string style = "";

        /// <summary>
        /// Style of this toolbar. Defaul; "Toolbar"
        /// Only need to be flagged on one item.
        /// </summary>
        public string Style
        {
            get { return style; }
            set { style = value; }
        }

        private bool label = false;

        /// <summary>
        /// Show or hide the toolbar label
        /// Only need to be flagged on one item.
        /// </summary>
        public bool Label
        {
            get { return label; }
            set { label = value; }
        }

        private bool flexible = false;

        /// <summary>
        /// This specific item will have a Flexible Space before
        /// </summary>
        public bool Flexible
        {
            get { return flexible; }
            set { flexible = value; }
        }

        private int priority = 0;

        /// <summary>
        /// Priority of this toolbar when sorting items. 
        /// Only need to be flagged on one item.
        /// </summary>
        public int Priority
        {
            get { return priority; }
            set { priority = value; }
        }

        public ToolbarAttribute(string name)
            : this(name, "", false, false, 0) { }

        public ToolbarAttribute(string name, int priority)
            : this(name, "", false, false, priority) { }

        public ToolbarAttribute(string name, string style)
            : this(name, style, false, false, 0) { }

        public ToolbarAttribute(string name, string style, int priority)
            : this(name, style, false, false, priority) { }

        public ToolbarAttribute(string name, bool label)
            : this(name, "", label, false, 0) { }

        public ToolbarAttribute(string name, bool label, int priority)
            : this(name, "", label, false, priority) { }

        public ToolbarAttribute(string name, string style, bool label)
            : this(name, style, label, false, 0) { }

        public ToolbarAttribute(string name, string style, bool label, int priority)
            : this(name, style, label, false, priority) { }

        public ToolbarAttribute(string name, string style, bool label, bool flexible)
            : this(name, style, label, flexible, 0) { }

        public ToolbarAttribute(string name, string style, bool label, bool flexible, int priority)
        {
            this.name = name;
            this.style = style;
            this.label = label;
            this.flexible = flexible;
            this.priority = priority;
        }
    }
}