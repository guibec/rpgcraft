namespace AdvancedInspector
{
    /// <summary>
    /// Define a menu item added when right-clicking a field.
    /// </summary>
    public interface IMenu : IRuntimeAttribute
    {
        /// <summary>
        /// Invoke the internal delegates when the menu item is selected.
        /// </summary>
        void Invoke(int index, object instance, object value);

        /// <summary>
        /// The name it would show up.
        /// </summary>
        string MenuItemName { get; }
    }
}
