using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;

namespace AdvancedInspector
{
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method)]
    public class MenuAttribute : Attribute, IListAttribute, IMenu
    {
        public delegate void MenuDelegate();
        public delegate void MenuStaticDelegate(MenuAttribute fieldMenuItem, object instance, object value);

        private string menuItemName;

        public string MenuItemName
        {
            get { return menuItemName; }
        }

        #region IRuntimeAttribute Implementation
        private string methodName = "";

        public string MethodName
        {
            get { return methodName; }
        }

        public Type Template
        {
            get { return typeof(MenuDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(MenuStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }

        public void Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return;
            
            try
            {
                if (delegates[index].Target == null)
                {
                   delegates[index].DynamicInvoke(this, instance, value);
                }
                else
                {
                    delegates[index].DynamicInvoke();
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method from a FieldMenuItem attribute failed. The exception was \"{0}\".", e.Message));
            }
        }
        #endregion

        public MenuAttribute(string menuItemName, string methodName)
        {
            this.menuItemName = menuItemName;
            this.methodName = methodName;
        }
    }
}
