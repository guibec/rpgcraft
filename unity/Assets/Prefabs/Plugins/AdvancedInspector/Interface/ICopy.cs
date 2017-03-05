using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// Give an object the power to handle it's own copying over an target destination.
    /// </summary>
    public interface ICopy
    {
        /// <summary>
        /// Should return a copy of itself. The overriden destination object is passed in case important fields are not to be replaced.
        /// </summary>
        object Copy(object destination);
    }
}
