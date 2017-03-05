using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample26_Toolbar : MonoBehaviour
    {
        // Toolbars are similar to groups.
        // However, they are not collapsible and display their inner fields horizontally. 
        // One thing you must explicitly declare is the name of the style you want to use for the toolbar and for the item.
        // Toolbars are always drawn in the header of the object, before any other fields.
        [Toolbar("MyToolbar", Style = "Toolbar"), Style("ToolbarTextField")]
        public float myField;

        // The Flexible attribute allows to add a flexible space between toolbar items.
        [Inspect(1), Toolbar("MyToolbar", Flexible = true), Style("ToolbarButton", Label = false)]
        public void MyMethod()
        {
            myField++;
        }
    }
}