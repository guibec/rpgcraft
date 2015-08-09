using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AdvancedInspector
{
    /// <summary>
    /// Give an object the power to detect if it can be clone to the target location.
    /// </summary>
    public interface ICopiable
    {
        /// <summary>
        /// Should return true if the object can be copied to replace the object destination.
        /// </summary>
        bool Copiable(object destination);
    }
}
