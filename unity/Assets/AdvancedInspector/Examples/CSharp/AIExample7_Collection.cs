using UnityEngine;
using System;
using System.Collections.Generic;

namespace AdvancedInspector
{
    public class AIExample7_Collection : MonoBehaviour
    {
        // Any collection inspected by Advanced Inspector has the re-ordering controls.
        [ReadOnly]
        public float[] myArray;

        // The collection attribute gives you control on how the collection is displayed.
        // Giving it a number forces the collection to be of a fixed size, it cannot grow or shrink.
        [Collection(10)]
        public int[] fixedArray;

        // You can also turn off the sortable feature of an array, if for example item should be listed in a specific order.
        [Collection(false)]
        public bool[] unsortableArray;

        // Collection can also be displayed 1 item at a time. You can use a drop down or buttons to navigate in it.
        [Collection(Display = CollectionDisplay.Button)]
        public List<bool> buttonList;

        [Collection(Display = CollectionDisplay.DropDown)]
        public List<bool> dropDownList;

        // A collection size and index can also be bound to an enum type.
        [Collection(typeof(MyCollectionEnum), Sortable = false)]
        public List<Vector3> enumBoundList;

        // Very large collection get a scrolling system to not display them all at once.
        [Collection(100, MaxDisplayedItems = 10)]
        public string[] largeCollection;

        // In some case, you may want a class to have a custom constructor.
        // Usually, Unity is unable to invoke that constructor.
        [Serializable]
        public class CustomConstructor
        {
            public string value;

            public CustomConstructor(string text)
            {
                value = text;
            }
        }

        [Constructor("InvokeConstructor")]
        public CustomConstructor[] constructors;

        public CustomConstructor InvokeConstructor()
        {
            return new CustomConstructor("This was added in a constructor");
        }

        [Collection("GetItemNames", Display = CollectionDisplay.DropDown)]
        public string[] items;

        private string[] GetItemNames()
        {
            return items;
        }

        [Serializable]
        public class CollectionContainer
        {
            public int integer;
            public string text;

            public override string ToString()
            {
                return text;
            }
        }

        [Collection(AlwaysExpanded = true)]
        public CollectionContainer[] alwaysExpanded;
    }

    // When binding an enum with a collection, the values should be in a zero-based 1 increment order, similar to the index of the collection.
    public enum MyCollectionEnum
    {
        ZeroValue = 0,
        FirstValue = 1,
        SecondValue = 2,
        ThirdValue = 3,
        ForthValue = 4
    }
}