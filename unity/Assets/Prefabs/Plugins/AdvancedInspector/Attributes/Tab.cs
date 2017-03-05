using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// The tabs allows to create a collection of tabs at the top based on an Enum's values.
    /// Hides or shows items that are part of the selected tab.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field | AttributeTargets.Method)]
    public class TabAttribute : Attribute, IRuntimeAttribute<Enum>
    {
        public delegate Enum DescriptorDelegate();
        public delegate Enum DescriptorStaticDelegate(TabAttribute descriptor, object instance, object value);

        private Enum tab;

        public Enum Tab
        {
            get { return tab; }
            set { tab = value; }
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

        public Enum Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return tab;

            try
            {
                if (delegates[index].Target == null)
                {
                    return (Enum)delegates[index].DynamicInvoke(this, instance, value);
                }
                else
                {
                    return (Enum)delegates[index].DynamicInvoke();
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method to retrieve a Tab attribute failed. The exception was \"{0}\".", e.Message));
                return null;
            }
        }
        #endregion

        public TabAttribute(object tab)
        {
            this.tab = (Enum)tab;
        }

        public TabAttribute(string methodName)
        {
            this.methodName = methodName;
        }
    }
}