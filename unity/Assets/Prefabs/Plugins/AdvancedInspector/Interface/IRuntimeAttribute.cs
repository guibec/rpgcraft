using System;
using System.Collections.Generic;

namespace AdvancedInspector
{
    /// <summary>
    /// Define an attribute that stores a method name
    /// Which should be turned into a delegate at runtime.
    /// </summary>
    public interface IRuntimeAttribute<T> : IRuntimeAttribute
    {
        /// <summary>
        /// Invoke the internal delegates and returns the requested values.
        /// T should be the same type as the Delegate return type.
        /// </summary>
        T Invoke(int index, object instance, object value);
    }

    public interface IRuntimeAttribute
    {
        /// <summary>
        /// Name of the MethodInfo to retrieve at runtime.
        /// </summary>
        string MethodName { get; }

        /// <summary>
        /// Prototype template of the delegate to create
        /// </summary>
        Type Template { get; }

        /// <summary>
        /// Prototype template for static external delegate
        /// </summary>
        Type TemplateStatic { get; }

        /// <summary>
        /// List of delegates to invoke.
        /// </summary>
        List<Delegate> Delegates { get; set; }
    }
}