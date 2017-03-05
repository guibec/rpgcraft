using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// The description of something; name, comment, url, etc.
    /// </summary>
    public class Description
    {
        private string name = "";

        /// <summary>
        /// Give this item a name.
        /// </summary>
        public string Name
        {
            get { return name; }
        }

        private string comment = "";

        /// <summary>
        /// Give this item a description. 
        /// Usually used for tooltip.
        /// </summary>
        public string Comment
        {
            get { return comment; }
        }

        private string url = "";

        /// <summary>
        /// Give this item an help URL.
        /// </summary>
        public string URL
        {
            get { return url; }
        }

        private Texture icon = null;

        /// <summary>
        /// Give this item an icon.
        /// Useful in a list of items.
        /// </summary>
        public Texture Icon
        {
            get { return icon; }
        }

        private Color color = Color.clear;

        /// <summary>
        /// Give this item a color.
        /// Default is transparent
        /// </summary>
        public Color Color
        {
            get { return color; }
        }

        public Description(string name)
            : this(name, "", "", null, Color.clear) { }

        public Description(string name, Texture icon)
            : this(name, "", "", icon, Color.clear) { }

        public Description(string name, Color color)
            : this(name, "", "", null, color) { }

        public Description(string name, Texture icon, Color color)
            : this(name, "", "", icon, color) { }

        public Description(string name, string comment)
            : this(name, comment, "", null, Color.clear) { }

        public Description(string name, string comment, Texture icon)
            : this(name, comment, "", icon, Color.clear) { }

        public Description(string name, string comment, Color color)
            : this(name, comment, "", null, color) { }

        public Description(string name, string comment, Texture icon, Color color)
            : this(name, comment, "", icon, color) { }

        public Description(string name, string comment, string url)
            : this(name, comment, url, null, Color.clear) { }

        public Description(string name, string comment, string url, Texture icon)
            : this(name, comment, url, icon, Color.clear) { }

        public Description(string name, string comment, string url, Color color)
            : this(name, comment, url, null, color) { }

        public Description(string name, string comment, string url, Texture icon, Color color)
        {
            this.name = name;
            this.comment = comment;
            this.url = url;
            this.icon = icon;
            this.color = color;
        }
    }

    /// <summary>
    /// Pairs an object with a description.
    /// Used by the Toolbox and the Advanced Inspector.
    /// </summary>
    public class DescriptionPair
    {
        private object value;

        public object Value
        {
            get { return value; }
        }

        private Description description;

        public Description Description
        {
            get { return description; }
        }

        public DescriptionPair(object value, Description descriptor)
        {
            this.value = value;
            this.description = descriptor;
        }

        public DescriptionPair(object value, string name)
            : this(value, new Description(name, "")) { }

        public DescriptionPair(object value, string name, string description)
            : this(value, new Description(name, description)) { }

        public static bool operator ==(DescriptionPair a, DescriptionPair b)
        {
            // If both are null, or both are same instance, return true.
            if (System.Object.ReferenceEquals(a, b))
                return true;

            // If one is null, but not both, return false.
            if (((object)a == null) || ((object)b == null))
                return false;

            return a.Equals(b);
        }

        public static bool operator !=(DescriptionPair a, DescriptionPair b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            DescriptionPair other = obj as DescriptionPair;
            if (other == null)
                return false;

            return this.Value.Equals(other.Value);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public static DescriptionPair GetDescription(object item)
        {
            if (item is DescriptionPair)
                return item as DescriptionPair;

            if (item != null)
                return new DescriptionPair(item, new Description(item.ToString(), ""));
            else
                return new DescriptionPair(null, new Description("None", ""));
        }

        public static IList<DescriptionPair> GetDescriptions(IList items)
        {
            List<DescriptionPair> pairs = new List<DescriptionPair>();
            foreach (object item in items)
                pairs.Add(GetDescription(item));

            return pairs;
        }
    }
}
