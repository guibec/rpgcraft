using System;

namespace AdvancedInspector
{
    /// <summary>
    /// Can only be placed a classed derived from FieldEditor, or a field/property taking a specific editor.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class FieldEditorAttribute : Attribute, IListAttribute
    {
        private string type = "";

        /// <summary>
        /// Type's name of the FieldEditor to use.
        /// </summary>
        public string Type
        {
            get { return type; }
        }

        public FieldEditorAttribute(string type)
        {
            this.type = type;
        }
    }
}