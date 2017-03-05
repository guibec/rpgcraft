using System;
using UnityEngine;
using System.Reflection;
using System.Collections.Generic;

namespace AdvancedInspector
{
    /// <summary>
    /// Changes the color of the background of an expandable item.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Class | AttributeTargets.Struct)]
    public class BackgroundAttribute : Attribute, IRuntimeAttribute<Color>
    {
        public delegate Color BackgroundDelegate();
        public delegate Color BackgroundStaticDelegate(BackgroundAttribute background, object instance, object value);

        private Color color = Color.clear;

        /// <summary>
        /// Give this item's background a color.
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
            set { methodName = value; }
        }

        public Type Template
        {
            get { return typeof(BackgroundDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(BackgroundStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }

        public Color Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return color;

            try
            {
                if (delegates[index].Target == null)
                {
                    return (Color)delegates[index].DynamicInvoke(this, instance, value);
                }
                else
                {
                    return (Color)delegates[index].DynamicInvoke();
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method to retrieve a Background attribute failed. The exception was \"{0}\".", e.Message));
                return color;
            }
        }
        #endregion

        public BackgroundAttribute(string methodName)
        {
            this.methodName = methodName;
        }

        public BackgroundAttribute(Delegate method)
        {
            this.delegates.Add(method);
        }

        public BackgroundAttribute(float r, float g, float b)
            : this(r, g, b, 1) { }

        public BackgroundAttribute(float r, float g, float b, float a)
        {
            this.color = new Color(r, g, b, a);
        }
    }
}