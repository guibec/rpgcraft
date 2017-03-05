using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// Returns the description of an item.
    /// Does not support multi-attribute, or multi-selection. First come, first serve.
    /// </summary>
    public interface IDescriptor
    {
        /// <summary>
        /// Note that instances/values may be null when getting data for a type.
        /// </summary>
        Description GetDescription(object[] instances, object[] values);
    }
}
