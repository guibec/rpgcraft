using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Prevent a nested object from having to be unfolded.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class DisplayAsParentAttribute : Attribute { }
}