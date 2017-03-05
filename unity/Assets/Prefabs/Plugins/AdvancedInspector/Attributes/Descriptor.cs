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
    public class DescriptorAttribute : Attribute, IRuntimeAttribute, IDescriptor
    {
        public delegate Description DescriptorDelegate();
        public delegate Description DescriptorStaticDelegate(DescriptorAttribute descriptor, object instance, object value);

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

        private string comment = "";

        /// <summary>
        /// Give this item a description. 
        /// Usually used for tooltip.
        /// </summary>
        public string Comment
        {
            get { return comment; }
            set { comment = value; }
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

        #region IDescriptor Implementation
        public Description GetDescription(object[] instances, object[] values)
        {
            if (delegates.Count == 0)
                return new Description(name, comment, url, icon, color);

            try
            {
                if (delegates[0].Target == null)
                {
                    return delegates[0].DynamicInvoke(this, instances[0], values[0]) as Description;
                }
                else
                {
                    return delegates[0].DynamicInvoke() as Description;
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
            this.comment = description;
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
            this.comment = description;
            this.icon = icon;
            this.color = color;
        }
        #endregion

#if !NETFX_CORE
        public static Description GetDescriptor(Type type)
        {
            object[] obj = type.GetCustomAttributes(typeof(IDescriptor), true);

            if (obj.Length == 0)
                return null;
            else
                return (obj[0] as IDescriptor).GetDescription(null, null);
        }

        public static List<Description> GetDescriptors(List<Type> types)
        {
            List<Description> descriptions = new List<Description>();
            foreach (Type type in types)
                descriptions.Add(GetDescriptor(type));

            return descriptions;
        }
#endif
    }
}