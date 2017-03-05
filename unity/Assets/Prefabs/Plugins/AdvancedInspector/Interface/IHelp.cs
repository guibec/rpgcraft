using System;
using System.Collections.Generic;

namespace AdvancedInspector
{
    /// <summary>
    /// Define an attribute that returns help information.
    /// Support multi-attributes on multi-selection.
    /// </summary>
    public interface IHelp
    {
        IList<HelpItem> GetHelp(object[] instances, object[] values);
    }
}
