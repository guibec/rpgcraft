using UnityEngine;
using System;
using System.Collections.Generic;

namespace AdvancedInspector
{
    public class AIExample22_DisplayAsParent : MonoBehaviour
    {
        // The DisplayAsParent attribute prevents a sub-object from being expandable
        // Instead it display its inner fields as being part of the parent.
        // This is useful to remove some depth in multi-nested-object.
        // The name of this field remains, but it shows no editor.
        [DisplayAsParent]
        public SubObject myObject = new SubObject();

        // You can also prevent the parent object from being hidden.
        // That way, it shows up as a title.
        [DisplayAsParent(false)]
        public SubObject myObjectWithTitle = new SubObject();

        [Serializable, AdvancedInspector(true, false)]
        public class SubObject
        {
            public int testInputField;
        }
    }
}