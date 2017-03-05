using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// Defines an attribute that can control if a node is in "read only" mode.
    /// Supports multiple attributes in a "If Any" fashion.
    /// </summary>
    public interface IReadOnly
    {
        bool IsReadOnly(object[] instances, object[] values);
    }
}
