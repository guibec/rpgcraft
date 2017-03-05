using UnityEngine;
using System;
using System.Collections;

namespace AdvancedInspector
{
    /// <summary>
    /// Removes the object picking field from a selectable object.
    /// Useful when the object is set internally, should be edited but not changed.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class NoPicker : Attribute, IPicker, IListAttribute 
    {
        public bool IsPickingAvailable(object[] instances, object[] values)
        {
            return false;
        }
    }
}
