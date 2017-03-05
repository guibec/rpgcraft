using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample3_InspectDynamic : MonoBehaviour
    {
        // The inspect attribute can be binded to a function.
        // This function should return true or false, if the item should be displayed or hidden.
        [Inspect("InspectItem")]
        public bool myVariable;

        // The function can be private or public, it doesn't matter.
        private bool InspectItem()
        {
            return displayItem;
        }

        private bool displayItem = true;

        // In this example, the button toggle on/off the display of "myVariable".
        [Inspect]
        public void PressMe()
        {
            displayItem = !displayItem;
        }
    }
}