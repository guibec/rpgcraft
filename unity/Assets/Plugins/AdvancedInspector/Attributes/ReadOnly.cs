using System;
using System.Collections.Generic;
using System.Reflection;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Makes a Property read only (cannot be modified)
    /// It's grayed out in the inspector, even if there's a setter.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method, AllowMultiple = false)]
    public class ReadOnlyAttribute : Attribute, IListAttribute, IRuntimeAttribute<bool>
    {
        public delegate bool ReadOnlyDelegate();
        public delegate bool ReadOnlyStaticDelegate(ReadOnlyAttribute readOnly, object instance, object value);

        #region IRuntime Implementation
        private string methodName = "";

        public string MethodName
        {
            get { return methodName; }
        }

        public Type Template
        {
            get { return typeof(ReadOnlyDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(ReadOnlyStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }

        public bool Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return true;

            try
            {
                if (delegates[index].Target == null)
                {
                    return (bool)delegates[index].DynamicInvoke(this, instance, value);
                }
                else
                {
                    return (bool)delegates[index].DynamicInvoke();
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method to retrieve a ReadOnly attribute failed. The exception was \"{0}\".", e.Message));
                return false;
            }
        }
        #endregion

        public ReadOnlyAttribute() { }

        public ReadOnlyAttribute(Delegate method)
        {
            this.delegates.Add(method);
        }

        public ReadOnlyAttribute(string methodName)
        {
            this.methodName = methodName;
        }
    }
}