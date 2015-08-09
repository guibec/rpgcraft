using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Define an exposed property that act as a object creator.
    /// The field gives the user the choices of all type deriving from that property type.
    /// In the case of a generic List, it offers way to add object in the list.
    /// If the list is of a value type (Ex.: int), it automaticly create an entry with the default value of that type.
    /// Field/Property's type sporting this attribute should derive from ComponentMonoBehaviour!
    /// Otherwise, Unity's serialization will kill the polymorphism involved.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class CreateDerivedAttribute : Attribute, IListAttribute { }
}