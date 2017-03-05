using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using System.Text;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Similar to Unity "Header" attribute, but can be place on any members and be set at runtime.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method)]
    public class TitleAttribute : Attribute, IRuntimeAttribute<TitleAttribute>
    {
        public delegate TitleAttribute TitleDelegate();
        public delegate TitleAttribute TitleStaticDelegate(TitleAttribute title, object instance, object value);

        private string message;

        /// <summary>
        /// The title message
        /// </summary>
        public string Message
        {
            get { return message; }
            set { message = value; }
        }

        private FontStyle style = FontStyle.Bold;

        /// <summary>
        /// The font style.
        /// </summary>
        public FontStyle Style
        {
            get { return style; }
            set { style = value; }
        }

        #region IRuntime Implementation
        private string methodName = "";

        public string MethodName
        {
            get { return methodName; }
        }

        public Type Template
        {
            get { return typeof(TitleDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(TitleStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }

        public TitleAttribute Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return this;

            try
            {
                if (delegates[index].Target == null)
                {
                    return delegates[0].DynamicInvoke(this, instance, value) as TitleAttribute;
                }
                else
                {
                    return delegates[0].DynamicInvoke() as TitleAttribute;
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method failed while trying to retrieve a Title attribute. The exception was \"{0}\".", e.Message));
                return null;
            }
        }
        #endregion

        public TitleAttribute(string methodName)
        {
            this.methodName = methodName;
        }

        public TitleAttribute(FontStyle style, string message)
        {
            this.style = style;
            this.message = message;
        }

        public TitleAttribute(Delegate method)
        {
            this.delegates.Add(method);
        }
    }
}
