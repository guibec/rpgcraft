using System;

namespace AdvancedInspector
{
    public delegate void AdvancedInspectorForceRefresh(bool rebuil);

    /// <summary>
    /// Turn off the default Inspector in favor or the Advanced one.
    /// If false, both may be draw if some members are flagged "Inspect", one after the other...
    /// </summary>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Interface, Inherited = true)]
    public class AdvancedInspectorAttribute : Attribute
    {
        /// <summary>
        /// The event fired when an out-of-context refresh is requested.
        /// Honestly, you should be using IDataChanged instead.
        /// </summary>
        public static event AdvancedInspectorForceRefresh OnForceRefresh;

        /// <summary>
        /// Static method use to force an out-of-context refresh of Advanced Inspector.
        /// If rebuild is true, its rebuilds all the InspectorField. It's a rather expensive operation.
        /// </summary>
        /// <param name="rebuild">Rebuild or not the field layout.</param>
        public static void Refresh(bool rebuild = false)
        {
            if (OnForceRefresh != null)
                OnForceRefresh(rebuild);
        }

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
