namespace AdvancedInspector
{
    /// <summary>
    /// Define an attribute that controls how many pixels is before and after an item.
    /// </summary>
    public interface ISpacing
    {
        /// <summary>
        /// Returns the number of spacing pixel after an item.
        /// </summary>
        int GetAfter(object[] instances, object[] values);

        /// <summary>
        /// Returns the number of spacing pixel before an item.
        /// </summary>
        int GetBefore(object[] instances, object[] values);
    }
}
