using System;
using System.Collections;
using System.Collections.Generic;

namespace AdvancedInspector
{
    /// <summary>
    /// Define an attribute that limits the potential values to a known collection.
    /// </summary>
    public interface IRestrict
    {
        /// <summary>
        /// Get a collection of every potential items.
        /// </summary>
        IList GetRestricted(object[] instances, object[] values);

        /// <summary>
        /// Define how the list should be displayed; drop down, button, checkboxes...
        /// </summary>
        RestrictDisplay GetDisplay(object[] instances, object[] values);

        /// <summary>
        /// Assuming the Display is set to button or check boxes, controls how many items is listed on a single row. 
        /// </summary>
        int GetItemsPerRow(object[] instances, object[] values);
    }
}
