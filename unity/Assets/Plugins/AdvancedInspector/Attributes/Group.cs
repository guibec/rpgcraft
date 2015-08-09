using System;
using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Allow to groups inspector items.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field | AttributeTargets.Method)]
    public class GroupAttribute : Attribute
    {
        private string name = "";

        /// <summary>
        /// Name of the group.
        /// </summary>
        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        private string description = "";

        /// <summary>
        /// Extra text for the group, displayed on the right side.
        /// </summary>
        public string Description
        {
            get { return description; }
            set { description = value; }
        }

        private string style = "";

        /// <summary>
        /// Style of this group.
        /// Only need to be flagged on one item.
        /// </summary>
        public string Style
        {
            get { return style; }
            set { style = value; }
        }

        private int priority = 0;

        /// <summary>
        /// Priority of this group when sorting items. 
        /// Only need to be flagged on one item.
        /// </summary>
        public int Priority
        {
            get { return priority; }
            set { priority = value; }
        }

        private bool expandable = true;

        /// <summary>
        /// If false, the group is always expanded and does not have an foldout arrow.
        /// </summary>
        public bool Expandable
        {
            get { return expandable; }
            set { expandable = value; }
        }

        private Color color = Color.clear;

        /// <summary>
        /// Give this item's background a color.
        /// </summary>
        public Color Color
        {
            get { return color; }
            set { color = value; }
        }

        public GroupAttribute(string name)
            : this(name, "", 0) { }

        public GroupAttribute(string name, int priority)
            : this(name, "", priority) { }

        public GroupAttribute(string name, string style)
            : this(name, style, 0) { }

        public GroupAttribute(string name, float r, float g, float b)
            : this(name, "", "", 0, r, g, b, 1) { }

        public GroupAttribute(string name, string style, int priority)
            : this(name, "", style, priority, 0, 0, 0, 0) { }

        public GroupAttribute(string name, string style, float r, float g, float b)
            : this(name, "", style, 0, r, g, b, 1) { }

        public GroupAttribute(string name, string style, int priority, float r, float g, float b)
            : this(name, "", style, priority, r, g, b, 1) { }

        public GroupAttribute(string name, string description, string style, int priority, float r, float g, float b, float a)
        {
            this.name = name;
            this.description = description;
            this.style = style;
            this.priority = priority;
            this.color = new Color(r, g, b, a);
        }
    }
}