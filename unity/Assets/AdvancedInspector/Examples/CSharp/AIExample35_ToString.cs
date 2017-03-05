using System;
using UnityEngine;

namespace AdvancedInspector
{
    public class AIExample35_ToString : MonoBehaviour
    {
        public Nested[] collection;

        [Serializable]
        public class Nested
        {
            public string name;

            // When overriding ToString, the AdvancedInspector display that string on the right side of the item when there is no picker.
            public override string ToString()
            {
                return name;
            }
        }
    }
}