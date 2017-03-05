using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Define an attribute that control the visibility of an inspector element.
    /// Supports multi-attribute in a OR manner where if any is hidden, all is hidden.
    /// </summary>
    public interface IVisibility
    {
        /// <summary>
        /// Return if the item is visible or not.
        /// </summary>
        bool IsItemVisible(object[] instances, object[] values);

        /// <summary>
        /// Return the inspection level of the item. 
        /// </summary>
        InspectorLevel GetItemLevel(object[] instances, object[] values);

        /// <summary>
        /// Return the sorting priority of the item. Lower values are displayed first. Default is 0.
        /// </summary>
        int GetItemPriority(object[] instances, object[] values);
    }
}
