using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// Return the type of the field assuming the field type is too generic for its editor.
    /// </summary>
    public interface IRuntimeType
    {
        /// <summary>
        /// Type should not be null.
        /// </summary>
        Type GetType(object[] instances, object[] values);
    }
}
