using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;

using UnityEngine;

namespace AdvancedInspector
{
    /// <summary>
    /// When a property is flagged this way, a help box is added after the inspector's field.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Method | AttributeTargets.Class | AttributeTargets.Struct, AllowMultiple = true)]
    public class HelpAttribute : Attribute, IRuntimeAttribute, IHelp
    {
        public const string IsNull = "HelpAttribute.IsValueNull";
        public const string IsNullOrEmpty = "HelpAttribute.IsStringNullOrEmpty";
        public const string IsMatch = "HelpAttribute.IsRegexMatch";

        public delegate HelpItem HelpDelegate();
        public delegate HelpItem HelpStaticDelegate(HelpAttribute help, object instance, object value);

        private HelpType type;

        /// <summary>
        /// Help type.
        /// Displays a specific icon.
        /// </summary>
        public HelpType Type
        {
            get { return type; }
            set { type = value; }
        }

        private string message;

        /// <summary>
        /// Help message.
        /// </summary>
        public string Message
        {
            get { return message; }
            set { message = value; }
        }

        private HelpPosition position = HelpPosition.After;
        
        /// <summary>
        /// By default, the helpbox is drawn after the field.
        /// If this is false, it is drawn before the field.
        /// </summary>
        public HelpPosition Position
        {
            get { return position; }
            set { position = value; }
        }

        private string regex;

        /// <summary>
        /// When using the IsRegex conditional, this string is used as a regular expresion.
        /// </summary>
        public string Regex
        {
            get { return regex; }
            set { regex = value; }
        }

        #region IHelp Implementation
        public IList<HelpItem> GetHelp(object[] instances, object[] values)
        {
            if (delegates.Count == 0)
                return new HelpItem[] { new HelpItem(type, message, position) };

            try
            {
                List<HelpItem> helps = new List<HelpItem>();
                for (int i = 0; i < delegates.Count; i++)
                {
                    HelpItem help = null;
                    if (delegates[i].Target == null)
                        help = delegates[i].DynamicInvoke(this, instances[i], values[i]) as HelpItem;
                    else
                        help = delegates[i].DynamicInvoke() as HelpItem;

                    if (help != null && !helps.Contains(help))
                        helps.Add(help);
                }

                return helps;
            }
            catch (Exception e)
            {
                if (e is TargetInvocationException)
                    e = ((TargetInvocationException)e).InnerException;

                Debug.LogError(string.Format("Invoking a method failed while trying to retrieve a Help attribute. The exception was \"{0}\".", e.Message));
                return new HelpItem[0];
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
            get { return typeof(HelpDelegate); }
        }

        public Type TemplateStatic
        {
            get { return typeof(HelpStaticDelegate); }
        }

        private List<Delegate> delegates = new List<Delegate>();

        public List<Delegate> Delegates
        {
            get { return delegates; }
            set { delegates = value; }
        }
        #endregion

        #region Constructors
        public HelpAttribute(string methodName)
            : this(methodName, HelpType.None, HelpPosition.After, "") { }

        public HelpAttribute(string methodName, HelpType type, string message)
            : this(methodName, type, HelpPosition.After, message) { }

        public HelpAttribute(HelpType type, string message)
            : this("", type, HelpPosition.After, message) { }

        public HelpAttribute(HelpType type, HelpPosition position, string message)
            : this("", type, position, message) { }

        public HelpAttribute(string methodName, HelpType type, HelpPosition position, string message)
        {
            this.methodName = methodName;
            this.type = type;
            this.position = position;
            this.message = message;
        }

        public HelpAttribute(Delegate method)
        {
            this.delegates.Add(method);
        }
        #endregion

        #region Static Help
        private static HelpItem IsValueNull(HelpAttribute help, object instance, object value)
        {
            if (value == null || (value is UnityEngine.Object && ((UnityEngine.Object)value) == null))
                return new HelpItem(help.type, help.message);

            return null;
        }

        private static HelpItem IsStringNullOrEmpty(HelpAttribute help, object instance, object value)
        { 
            if (value is string && string.IsNullOrEmpty((string)value))
                return new HelpItem(help.type, help.message);

            return null;
        }

        private static HelpItem IsRegexMatch(HelpAttribute help, object instance, object value)
        {
            if (value == null)
                return null;

            string text = value.ToString();
            if (System.Text.RegularExpressions.Regex.IsMatch(text, help.regex))
                return new HelpItem(help.type, help.message);

            return null;
        }
        #endregion
    }

    /// <summary>
    /// Because the internal enum for help display is Editor only.
    /// </summary>
    public enum HelpType
    {
        None = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
    }

    /// <summary>
    /// The position where the help box should be placed.
    /// </summary>
    public enum HelpPosition
    { 
        After,
        Before
    }
}