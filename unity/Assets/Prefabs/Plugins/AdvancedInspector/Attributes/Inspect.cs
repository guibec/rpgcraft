using System;
using System.Reflection;
using System.Collections.Generic;
using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// Makes a property viewable in the Inspector of Unity.
    /// Turns a method into a button in the Inspector.
    /// You can input a conditional statement for your property to show up or not.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method)]
    public class InspectAttribute : Attribute, IRuntimeAttribute, IVisibility
    {
        public delegate bool InspectDelegate();
        public delegate bool InspectStaticDelegate(InspectAttribute inspect, object instance, object value);

        private InspectorLevel level;

        /// <summary>
        /// Inspector level are use to hide more advanced item.
        /// Items under the current Inspector levels are hidden.
        /// </summary>
        public InspectorLevel Level
        {
            get { return level; }
            set { level = value; }
        }

        private bool condition = true;

        /// <summary>
        /// Inverse the condition used by the IRuntime method.
        /// </summary>
        public bool Condition
        {
            get { return condition; }
            set { condition = value; }
        }

        private int priority = 0;

        /// <summary>
        /// Priority of display of this item.
        /// Smaller values are displayed first. Negative value are supported. Default is 0.
        /// </summary>
        public int Priority
        {
            get { return priority; }
            set { priority = value; }
        }

        #region IVisibility Implementation
        public bool IsItemVisible(object[] instances, object[] values)
        {
            if (delegates.Count == 0)
                return true;

            try
            {
                for (int i = 0; i < delegates.Count; i++)
                {
                    if (i >= instances.Length)
                        break;

                    bool visible = false;
                    if (delegates[i].Target == null)
                    {
                        if (condition)
                            visible = (bool)delegates[i].DynamicInvoke(this, instances[i], values[i]);
                        else
                            visible = !(bool)delegates[i].DynamicInvoke(this, instances[i], values[i]);
                    }
                    else
                    {
                        if (condition)
                            visible = (bool)delegates[i].DynamicInvoke();
                        else
                            visible = !(bool)delegates[i].DynamicInvoke();
                    }

                    if (visible)
                        return true;
                }

                return false;
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method to retrieve a Inspect attribute failed. The exception was \"{0}\".", e.Message));
                return false;
            }
        }

        public InspectorLevel GetItemLevel(object[] parents, object[] values)
        {
            return level;
        }

        public int GetItemPriority(object[] parents, object[] values)
        {
            return priority;
        }
        #endregion

        #region IRuntime Implementation
        private string methodName = "";

        public string MethodName
        {
            get { return methodName; }
            set { methodName = value; }
        }

        public Type Template
        {
            get { return typeof(InspectDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(InspectStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }
        #endregion

        #region Compile Time Constructor
        public InspectAttribute()
            : this(InspectorLevel.Basic, "", true, 0) { }

        public InspectAttribute(int priority)
            : this(InspectorLevel.Basic, "", true, priority) { }

        public InspectAttribute(InspectorLevel level)
            : this(level, "", true, 0) { }

        public InspectAttribute(InspectorLevel level, int priority)
            : this(level, "", true, priority) { }

        public InspectAttribute(InspectorLevel level, string methodName)
            : this(level, methodName, true, 0) { }

        public InspectAttribute(InspectorLevel level, string methodName, int priority)
            : this(level, methodName, true, priority) { }

        public InspectAttribute(InspectorLevel level, string methodName, bool condition)
            : this(level, methodName, condition, 0) { }

        public InspectAttribute(string methodName)
            : this(InspectorLevel.Basic, methodName, true, 0) { }

        public InspectAttribute(string methodName, int priority)
            : this(InspectorLevel.Basic, methodName, true, priority) { }

        public InspectAttribute(string methodName, bool condition)
            : this(InspectorLevel.Basic, methodName, condition, 0) { }

        public InspectAttribute(string methodName, bool condition, int priority)
            : this(InspectorLevel.Basic, methodName, condition, priority) { }

        public InspectAttribute(InspectorLevel level, string methodName, bool condition, int priority)
        {
            this.level = level;
            this.condition = condition;
            this.methodName = methodName;
            this.priority = priority;
        }
        #endregion

        #region Runtime Constructor
        public InspectAttribute(Delegate method)
            : this(InspectorLevel.Basic, method, true, 0) { }

        public InspectAttribute(Delegate method, int priority)
            : this(InspectorLevel.Basic, method, true, priority) { }

        public InspectAttribute(Delegate method, bool condition)
            : this(InspectorLevel.Basic, method, condition, 0) { }

        public InspectAttribute(Delegate method, bool condition, int priority)
            : this(InspectorLevel.Basic, method, condition, priority) { }

        public InspectAttribute(InspectorLevel level, Delegate method)
            : this(level, method, true, 0) { }

        public InspectAttribute(InspectorLevel level, Delegate method, int priority)
            : this(level, method, true, priority) { }

        public InspectAttribute(InspectorLevel level, Delegate method, bool condition, int priority)
        {
            this.level = level;
            this.condition = condition;
            this.priority = priority;
            this.delegates.Add(method);
        }
        #endregion
    }
}