using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Since internal Unity classes are not "Advanced Inspector" friendly, 
    /// this attribute force their own members to be exposed without the need of "InspectAttribute".
    /// Be careful, all public property/fields will be exposed in a recursive manner. 
    /// This may expose stuff that were not meant to be exposed.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class BypassAttribute : Attribute, IListAttribute { }
}