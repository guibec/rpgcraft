using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// Defined an interface that controls if an object field allows its reference to be changed.
    /// </summary>
    public interface IPicker
    {
        bool IsPickingAvailable(object[] instances, object[] values);
    }
}
