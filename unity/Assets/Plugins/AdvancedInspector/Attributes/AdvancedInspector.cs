using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Turn off the default Inspector in favor or the Advanced one.
    /// If false, both may be draw if some members are flagged "Inspect", one after the other...
    /// </summary>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Interface, Inherited = true)]
    public class AdvancedInspectorAttribute : Attribute
    {
        private bool inspectDefaultItems = false;

        /// <summary>
        /// If true, the Advanced Inspector inspect all the item the default Inspector does without adding the [Inspect] attribute.
        /// You can still add item that Unity would not display by adding the [Inspect] attribute.
        /// </summary>
        public bool InspectDefaultItems
        {
            get { return inspectDefaultItems; }
            set { inspectDefaultItems = value; }
        }

        private bool showScript = true;

        /// <summary>
        /// Show or hide the script field at the top of the inspector.
        /// The script field allow to change the type of the object.
        /// </summary>
        public bool ShowScript
        {
            get { return showScript; }
            set { showScript = value; }
        }

        private bool expandable = true;

        /// <summary>
        /// Is this object expandable in a in-lined context?
        /// </summary>
        public bool Expandable
        {
            get { return expandable; }
            set { expandable = value; }
        }

        public AdvancedInspectorAttribute() { }

        public AdvancedInspectorAttribute(bool inspectDefaultItems)
        {
            this.inspectDefaultItems = inspectDefaultItems;
        }

        public AdvancedInspectorAttribute(bool inspectDefaultItems, bool showScript)
        {
            this.showScript = showScript;
            this.inspectDefaultItems = inspectDefaultItems;
        }
    }
}
