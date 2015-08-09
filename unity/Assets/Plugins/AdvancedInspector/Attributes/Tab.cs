using System;

namespace AdvancedInspector
{
    /// <summary>
    /// The tabs allows to create a collection of tabs at the top based on an Enum's values.
    /// Hides or shows items that are part of the selected tab.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field | AttributeTargets.Method)]
    public class TabAttribute : Attribute
    {
        private Enum tab;

        public Enum Tab
        {
            get { return tab; }
            set { tab = value; }
        }

        public TabAttribute(object tab)
        {
            this.tab = (Enum)tab;
        }
    }
}