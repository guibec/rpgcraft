using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Forces a field to display a FieldEditor related to its current runtime type instead of the field type.
    /// The Runtime version supply the type itself. Useful when the field value is null or for an unknown object picker.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class RuntimeResolveAttribute : Attribute, IListAttribute, IRuntimeAttribute<Type>, IRuntimeType
    {
        public delegate Type RuntimeResolveDelegate();
        public delegate Type RuntimeResolveStaticDelegate(RuntimeResolveAttribute runtimeResolve, object instance, object value);

        #region IReadOnly Implementation
        public Type GetType(object[] instances, object[] values)
        {
            if (delegates.Count == 0)
            {
                for (int i = 0; i < values.Length; i++)
                {
                    if (values[0] == null)
                        continue;

                    return values[0].GetType();
                }
            }

            Type type = null;
            for (int i = 0; i < delegates.Count; i++)
            {
                Type next = Invoke(i, instances[i], values[i]);
                if (type != null && type != next)
                    return null;
                else
                    type = next;
            }

            return type;
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
            get { return typeof(RuntimeResolveDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(RuntimeResolveStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }

        public Type Invoke(int index, object instance, object value)
        {
            if (delegates.Count == 0 || index >= delegates.Count)
                return null;

            try
            {
                if (delegates[index].Target == null)
                {
                    return delegates[index].DynamicInvoke(this, instance, value) as Type;
                }
                else
                {
                    return delegates[index].DynamicInvoke() as Type;
                }
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method from a RuntimeResolve attribute failed. The exception was \"{0}\".", e.Message));
                return null;
            }
        }
        #endregion

        public RuntimeResolveAttribute() { }

        public RuntimeResolveAttribute(string methodName)
        {
            this.methodName = methodName;
        }

        public RuntimeResolveAttribute(Delegate method)
        {
            this.delegates.Add(method);
        }
    }
}