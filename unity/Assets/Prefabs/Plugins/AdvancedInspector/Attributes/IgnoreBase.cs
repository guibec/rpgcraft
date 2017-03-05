using System;

namespace AdvancedInspector
{
    /// <summary>
    /// When deriving from Unity's base type (Like Button) and you wish to add your own inspection on top while ignoring theirs, flag you type with it.
    /// Note; you may need to create an empty InspectorEditor for your type.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class)]
    public class IgnoreBase : Attribute { }
}
