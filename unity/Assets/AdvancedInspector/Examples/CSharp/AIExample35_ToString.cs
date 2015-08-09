using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using UnityEngine;
using AdvancedInspector;

[AdvancedInspector]
public class AIExample35_ToString : MonoBehaviour
{
    [Inspect]
    public Nested[] collection;

    [Serializable]
    public class Nested
    {
        [Inspect]
        public string name;

        // When overriding ToString, the AdvancedInspector display that string on the right side of the item when there is no picker.
        public override string ToString()
        {
            return name;
        }
    }
}
