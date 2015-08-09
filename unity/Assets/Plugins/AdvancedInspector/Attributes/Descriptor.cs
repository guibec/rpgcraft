using System;
using System.Collections.Generic;
using System.Reflection;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// A description is the information about "something".
    /// It contains an optional name, description, icon, color.
    /// It can be used both as a attributes or a normal object, container of information.
    /// Ex.: The Toolbox is using it as object.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method | AttributeTargets.Class | 
        AttributeTargets.Interface | AttributeTargets.Struct | AttributeTargets.Enum, Inherited = false)]
    public class DescriptorAttribute : Attribute, IRuntimeAttribute<DescriptorAttribute>
    {
        public delegate DescriptorAttribute DescriptorDelegate();
        public delegate DescriptorAttribute DescriptorStaticDelegate(DescriptorAttribute descriptor, object instance, object value);

        private static Color TRANSPARENT = new Color(0, 0, 0, 0);

        private string name = "";

        /// <summary>
        /// Give this item a name.
        /// </summary>
        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        private string description = "";

        /// <summary>
        /// Give this item a description. 
        /// Usually used for tooltip.
        /// </summary>
        public string Description
        {
            get { return description; }
            set { description = value; }
        }

        private string url = "";

        /// <summary>
        /// Give this item an help URL.
        /// </summary>
        public string URL
        {
            get { return url; }
            set { url = value; }
        }

        private Texture icon = null;

        /// <summary>
        /// Give this item an icon.
        /// Useful in a list of items.
        /// </summary>
        public Texture Icon
        {
            get { return icon; }
            set { icon = value; }
        }

        private Color color = Color.clear;

        /// <summary>
        /// Give this item a color.
        /// Default is transparent
        /// </summary>
        public Color Color
        {
            get { return color; }
            set { color = value; }
        }

        #region IRuntime Implementation
        private string methodName = "";

        public string MethodName
        {
            get { return methodName; }
        }

        public Type Template
        {
            get { return typeof(DescriptorDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(DescriptorStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }

        public DescriptorAttribute Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return this;

            try
            {
                if (delegates[index].Target == null)
                {
                    return delegates[0].DynamicInvoke(this, instance, value) as DescriptorAttribute;
                }
                else
                {
                    return delegates[0].DynamicInvoke() as DescriptorAttribute;
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method to retrieve a Destriptor attribute failed. The exception was \"{0}\".", e.Message));
                return null;
            }
        }
        #endregion

        public DescriptorAttribute() { }

        #region Attributes Constructor
        public DescriptorAttribute(string methodName)
        {
            this.methodName = methodName;
        }

        public DescriptorAttribute(float r, float g, float b)
            : this("", "", "", r, g, b, 1) { }

        public DescriptorAttribute(string name, string description)
            : this(name, description, "", 0, 0, 0, 0) { }

        public DescriptorAttribute(string name, string description, string url)
            : this(name, description, url, 0, 0, 0, 0) { }

        public DescriptorAttribute(string name, string description, string url, float r, float g, float b)
            : this(name, description, url, r, g, b, 1) { }

        private DescriptorAttribute(string name, string description, string url, float r, float g, float b, float a)
        {
            this.name = name;
            this.description = description;
            this.url = url;
            color = new Color(r, g, b, a);
        }
        #endregion

        #region Object Constructor
        public DescriptorAttribute(string name, string description, Texture icon)
            : this(name, description, icon, TRANSPARENT) { }

        public DescriptorAttribute(string name, string description, Texture icon, Color color)
        {
            this.name = name;
            this.description = description;
            this.icon = icon;
            this.color = color;
        }
        #endregion

        public static DescriptorAttribute GetDescriptor(Type type)
        {
            object[] obj = type.GetCustomAttributes(typeof(DescriptorAttribute), true);

            if (obj.Length == 0)
                return null;
            else
                return (obj[0] as DescriptorAttribute);
        }

        public static List<DescriptorAttribute> GetDescriptors(List<Type> types)
        {
            List<DescriptorAttribute> descriptors = new List<DescriptorAttribute>();

            foreach (Type type in types)
                descriptors.Add(GetDescriptor(type));

            return descriptors;
        }
    }

    /// <summary>
    /// Pairs an object with a descriptor.
    /// Used by the Toolbox and the Advanced Inspector.
    /// </summary>
    public class DescriptorPair
    {
        private object value;

        public object Value
        {
            get { return value; }
        }

        private DescriptorAttribute descriptor;

        public DescriptorAttribute Descriptor
        {
            get { return descriptor; }
        }

        public DescriptorPair(object value, DescriptorAttribute descriptor)
        {
            this.value = value;
            this.descriptor = descriptor;
        }

        public DescriptorPair(object value, string name)
            : this(value, new DescriptorAttribute(name, "")) { }

        public DescriptorPair(object value, string name, string description)
            : this(value, new DescriptorAttribute(name, description)) { }

        public static bool operator ==(DescriptorPair a, DescriptorPair b)
        {
            // If both are null, or both are same instance, return true.
            if (System.Object.ReferenceEquals(a, b))
                return true;

            // If one is null, but not both, return false.
            if (((object)a == null) || ((object)b == null))
                return false;

            return a.Equals(b);
        }

        public static bool operator !=(DescriptorPair a, DescriptorPair b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            DescriptorPair other = obj as DescriptorPair;
            if (other == null)
                return false;

            return this.Value.Equals(other.Value);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }
}