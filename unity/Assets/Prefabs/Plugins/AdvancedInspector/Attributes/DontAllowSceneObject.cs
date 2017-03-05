using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Prevent Scene Object from being browsed in a Object property.
    /// By default, scene and asset are displayed.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class DontAllowSceneObjectAttribute : Attribute, IListAttribute { }
}