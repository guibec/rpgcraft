using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

using AdvancedInspector;

[AdvancedInspector]
public class AIExample7_Collection : MonoBehaviour 
{
    // Any collection inspected by Advanced Inspector has the re-ordering controls.
    [Inspect]
    public float[] myArray;

    // The collection attribute gives you control on how the collection is displayed.
    // Giving it a number forces the collection to be of a fixed size, it cannot grow or shrink.
    [Inspect, Collection(10)]
    public int[] fixedArray;

    // You can also turn off the sortable feature of an array, if for example item should be listed in a specific order.
    [Inspect, Collection(false)]
    public bool[] unsortableArray;

    // Collection can also be displayed 1 item at a time. You can use a drop down or buttons to navigate in it.
    [Inspect, Collection(Display = CollectionDisplay.DropDown)]
    public List<bool> dropDownList;

    // A collection size and index can also be bound to an enum type.
    [Inspect, Collection(typeof(MyCollectionEnum))]
    public List<Vector3> enumBoundList;

    // Very large collection get a scrolling system to not display them all at once.
    [Inspect, Collection(100, MaxDisplayedItems = 10)]
    public string[] largeCollection;

    [Inspect, Expandable(false)]
    public Transform myTransform;

    // In some case, you may want a class to have a custom constructor.
    // Usually, Unity is unable to invoke that constructor.
    [Serializable]
    public class CustomConstructor
    {
        [Inspect]
        public string value;

        public CustomConstructor(string text)
        {
            value = text;
        }
    }

    [Inspect, Constructor("InvokeConstructor")]
    public CustomConstructor[] constructors;

    public CustomConstructor InvokeConstructor()
    {
        return new CustomConstructor("This was added in a constructor");
    }
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