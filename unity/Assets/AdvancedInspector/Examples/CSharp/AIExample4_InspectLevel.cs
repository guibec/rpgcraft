using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample4_InspectLevel : MonoBehaviour
    {
        // Item can be displayed in "levels".
        // Advanced Inspector contains 3 levels; Basic, Advanced, Debug.
        [Inspect(InspectorLevel.Basic)]
        public bool basicVariable;

        // Higher level can be displayed in the inspector by right-clicking on a label and selecting that level.
        [Inspect(InspectorLevel.Advanced)]
        public bool advancedVariable;

        // It allows to hide items that are only useful for debug purpose.
        [Inspect(InspectorLevel.Debug)]
        public bool debugVariable;
    }
}